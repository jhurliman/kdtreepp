"""Test consumption through a generated registry, with no local_path_override."""
from functools import partial
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import os
from pathlib import Path
import re
import subprocess
import tempfile
from threading import Thread
from prepare_bcr import prepare

root = Path(__file__).resolve().parents[1]
version = re.search(r'version\s*=\s*"([^"]+)"', (root / 'MODULE.bazel').read_text()).group(1)
with tempfile.TemporaryDirectory(prefix='kdtree-bcr-') as temp:
    base = Path(temp)
    archive = base / 'source.tar.gz'
    subprocess.run(['git','archive','--format=tar.gz',f'--prefix=kdtreepp-{version}/',f'--output={archive}','HEAD'],cwd=root,check=True)
    server = ThreadingHTTPServer(('127.0.0.1',0),partial(SimpleHTTPRequestHandler,directory=temp))
    thread = Thread(target=server.serve_forever,daemon=True)
    thread.start()
    try:
        registry = base / 'registry'
        actual = prepare(archive,registry,f'http://127.0.0.1:{server.server_port}/source.tar.gz')
        assert actual == version
        descriptor = registry / 'bazel_registry.json'
        assert descriptor.read_text() == '{}\n'
        existing = '{"mirrors": []}\n'
        descriptor.write_text(existing)
        prepare(archive, registry, f'http://127.0.0.1:{server.server_port}/source.tar.gz')
        assert descriptor.read_text() == existing, 'existing registry configuration must survive'

        consumer = base / 'consumer'
        consumer.mkdir()
        (consumer/'MODULE.bazel').write_text(f'module(name="registry_consumer")\nbazel_dep(name="kdtreepp",version="{version}",repo_name="spatial")\nbazel_dep(name="rules_cc",version="0.2.22")\n')
        (consumer/'BUILD.bazel').write_text('load("@rules_cc//cc:cc_test.bzl","cc_test")\ncc_test(name="consumer",srcs=["main.cpp"],deps=["@spatial//:kdtreepp"])\n')
        (consumer/'main.cpp').write_bytes((root/'examples/cmake-consumer/main.cpp').read_bytes())
        (consumer/'.bazelversion').write_bytes((root/'.bazelversion').read_bytes())
        subprocess.run([os.environ.get('BAZEL','bazel'),'--batch','test','//:consumer','--cxxopt=-std=c++17',f'--registry={registry.as_uri()}','--registry=https://bcr.bazel.build'],cwd=consumer,check=True)
    finally:
        server.shutdown()
        server.server_close()
