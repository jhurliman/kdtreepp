"""Generate a registry entry from the exact source archive being released."""
import argparse
import base64
import hashlib
import json
from pathlib import Path
import re
import tarfile


def prepare(archive, output, url=None):
    root = Path(__file__).resolve().parents[1]
    with tarfile.open(archive, 'r:gz') as tar:
        modules = [m for m in tar.getmembers() if m.name.count('/') == 1 and m.name.endswith('/MODULE.bazel')]
        if len(modules) != 1 or not modules[0].isfile() or modules[0].size > 65536:
            raise ValueError('archive must contain one top-level MODULE.bazel')
        module = tar.extractfile(modules[0]).read()
        text = module.decode('utf-8')
        name = re.search(r'\bname\s*=\s*"([a-z0-9_]+)"', text).group(1)
        version = re.search(r'\bversion\s*=\s*"([0-9]+\.[0-9]+\.[0-9]+)"', text).group(1)
        if name != 'kdtreepp':
            raise ValueError('unexpected module name')
        prefix = modules[0].name.split('/')[0]
        if prefix != 'kdtreepp-' + version:
            raise ValueError('archive prefix must match the module version')
        for required in ['BUILD.bazel', 'LICENSE', 'include/kdtreepp.hpp']:
            if not tar.getmember(prefix + '/' + required).isfile():
                raise ValueError('missing release file: ' + required)
    directory = Path(output) / 'modules' / name
    target = directory / version
    target.mkdir(parents=True, exist_ok=True)
    metadata_path = directory / 'metadata.json'
    metadata = json.loads(metadata_path.read_text()) if metadata_path.exists() else json.loads((root / '.bcr/metadata.template.json').read_text())
    if version not in metadata['versions']:
        metadata['versions'].append(version)
    metadata_path.write_text(json.dumps(metadata, indent=2) + '\n')
    source = {
        'url': url or f'https://github.com/jhurliman/kdtreepp/archive/refs/tags/v{version}.tar.gz',
        'integrity': 'sha256-' + base64.b64encode(hashlib.sha256(Path(archive).read_bytes()).digest()).decode(),
        'strip_prefix': prefix,
    }
    (target / 'source.json').write_text(json.dumps(source, indent=2) + '\n')
    (target / 'MODULE.bazel').write_bytes(module)
    (target / 'presubmit.yml').write_bytes((root / '.bcr/presubmit.yml').read_bytes())
    descriptor = Path(output) / 'bazel_registry.json'
    if not descriptor.exists():
        descriptor.write_text('{}\n')
    return version


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('archive', type=Path)
    parser.add_argument('--output', type=Path, required=True)
    parser.add_argument('--url')
    args = parser.parse_args()
    print(prepare(args.archive, args.output, args.url))
