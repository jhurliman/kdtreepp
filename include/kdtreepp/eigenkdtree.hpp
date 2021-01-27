#include <Eigen/Dense>
#include <variant>
#include <vector>

namespace kdtreepp {

// Define an alignment boundary equal to 4 times the size of the underlying
// scalar type T times the number of dimensions N.
template <typename T, int N>
constexpr size_t ALIGNMENT = sizeof(T) * 4 * (1 + ((N - 1) / 4));

// The EigenKdTreeNode represents one element of the k-d tree, which may be
// either a leaf (containing an iterable sequence), or a branch (containing a
// left and right child). All nodes have a bounding box.
//
// The node is constructed with an iterable range and two function objects.
// The first transforms the dereferenced iterator into a sortable point (i.e.
// the element centroid), and the second returns a point or aligned box that is
// used to extend a bounding box.
template <typename Iterator, typename T, int N>
class alignas(ALIGNMENT<T, N>) EigenKdTreeNode {
public:
  // Construct a EigenKdTreeNode from the iterable range.
  // The node will compute a surrounding bounds for the range of objects,
  // based on a bounding element returned from the "boundsGetter" functor. This
  // element can be either a point or a box. If the node has too many elements
  // (based on maxPerLeaf), and can be subdivided (based on maxSubDivs), it will
  // median-sort, in-place, the iterator range around a midpoint chosen by
  // comparing the "sort point" of the elements along the major axis dimension.
  //
  // This function *will* allocate memory for the child nodes, but does
  // not require any allocation for the contained objects.
  template <typename SortPointGetter, typename BoundsGetter>
  explicit EigenKdTreeNode(Iterator itemBegin, Iterator itemEnd, SortPointGetter&& sortPointGetter,
                           BoundsGetter&& boundsGetter, const int maxPerLeaf = 8,
                           const int maxSubDivs = 16) {
    const auto count = std::distance(itemBegin, itemEnd);

    // Compute the bounds
    _bounds.setEmpty();
    for (auto iter = itemBegin; iter != itemEnd; ++iter) {
      _bounds.extend(boundsGetter(*iter));
    }

    if (maxSubDivs > 0 && count > static_cast<decltype(count)>(maxPerLeaf)) {
      // Subdivide
      int majorAxis = 0;
      _bounds.sizes().maxCoeff(&majorAxis);
      Iterator itemMid = itemBegin;
      std::advance(itemMid, count / 2);

      // Median-sort items according to major axis of bounding box
      std::nth_element(itemBegin, itemMid, itemEnd,
                       [majorAxis, &sortPointGetter](auto& a, auto& b) -> bool {
                         return sortPointGetter(a)[majorAxis] < sortPointGetter(b)[majorAxis];
                       });

      // Create a branch
      _body = Branch{std::make_unique<this_type>(
                         itemBegin, itemMid, std::forward<SortPointGetter>(sortPointGetter),
                         std::forward<BoundsGetter>(boundsGetter), maxPerLeaf, maxSubDivs - 1),
                     std::make_unique<this_type>(
                         itemMid, itemEnd, std::forward<SortPointGetter>(sortPointGetter),
                         std::forward<BoundsGetter>(boundsGetter), maxPerLeaf, maxSubDivs - 1)};
    } else {
      _body = Leaf{itemBegin, itemEnd};
    }
  }

  bool isLeaf() const { return _body.index() == 0; }

  bool isBranch() const { return _body.index() == 1; }

  const auto& bounds() const { return _bounds; }

  // Recursively visit every item in this node and its children,
  // provided the bounding box of each node passes the "BoundsTest" function
  // passed in here - BoundsTest should take a const bounds as an argument and
  // return a bool, based on whether we should consider the node.
  //
  // Once leaf nodes are reached, the iteration range with the leaf will
  // be iterated over, and every item therein will be passed to visitor
  template <typename BoundsTest, typename Visitor>
  void visit(BoundsTest&& boundsTest, Visitor&& visitor) {
    if (!boundsTest(_bounds)) {
      return;
    }

    if (isLeaf()) {
      auto& leaf = std::get<0>(_body);
      for (auto iter = leaf.begin; iter != leaf.end; ++iter) {
        visitor(*iter);
      }
    } else if (isBranch()) {
      auto& branch = std::get<1>(_body);
      branch.left->visit(std::forward<BoundsTest>(boundsTest), std::forward<Visitor>(visitor));
      branch.right->visit(std::forward<BoundsTest>(boundsTest), std::forward<Visitor>(visitor));
    }
  }

  // Recursively visit every item in this node and its children,
  // provided the bounding box of each node passes the "BoundsTest" function
  // passed in here - BoundsTest should take a const bounds as an argument and
  // return a bool, based on whether we should consider the node.
  //
  // Once leaf nodes are reached, the iteration range with the leaf will
  // be iterated over, and every item therein will be passed to visitor
  //
  // (const version)
  template <typename BoundsTest, typename Visitor>
  void visit(BoundsTest&& boundsTest, Visitor&& visitor) const {
    if (!boundsTest(_bounds)) {
      return;
    }

    if (isLeaf()) {
      const auto& leaf = std::get<0>(_body);
      for (auto iter = leaf.begin; iter != leaf.end; ++iter) {
        visitor(*iter);
      }
    } else if (isBranch()) {
      const auto& branch = std::get<1>(_body);
      branch.left->visit(std::forward<BoundsTest>(boundsTest), std::forward<Visitor>(visitor));
      branch.right->visit(std::forward<BoundsTest>(boundsTest), std::forward<Visitor>(visitor));
    }
  }

private:
  using this_type = EigenKdTreeNode<Iterator, T, N>;
  Eigen::AlignedBox<T, N> _bounds;

  struct Leaf {
    Iterator begin;
    Iterator end;
  };

  struct Branch {
    std::unique_ptr<this_type> left;
    std::unique_ptr<this_type> right;
  };

  std::variant<Leaf, Branch> _body;
};

template <typename T, int N, typename Iterator, typename SortPointGetter, typename BoundsGetter>
EigenKdTreeNode<Iterator, T, N> MakeEigenKdTreeNode(Iterator itemBegin, Iterator itemEnd,
                                                    SortPointGetter&& sortPointGetter,
                                                    BoundsGetter&& boundsGetter,
                                                    const int maxPerLeaf = 8,
                                                    const int maxSubDivs = 16) {
  return EigenKdTreeNode<Iterator, T, N>{itemBegin,
                                         itemEnd,
                                         std::forward<SortPointGetter>(sortPointGetter),
                                         std::forward<BoundsGetter>(boundsGetter),
                                         maxPerLeaf,
                                         maxSubDivs};
}

}  // namespace kdtreepp
