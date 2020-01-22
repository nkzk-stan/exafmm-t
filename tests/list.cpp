#include <cassert>
#include "args.h"
#include "build_list.h"
#include "build_tree.h"
#include "dataset.h"
#include "geometry.h"
#include "timer.h"
#include "test.h"

using namespace exafmm_t;

int main(int argc, char** argv) {
  Args args(argc, argv);
  Bodies<real_t> sources = init_sources<real_t>(args.numBodies, args.distribution, 0);
  Bodies<real_t> targets = init_targets<real_t>(args.numBodies, args.distribution, 5);
  
  TestFMM fmm(args.ncrit);  // p and nsurf are set to 1 in the constructor

  NodePtrs<real_t> leafs, nonleafs;
  get_bounds(sources, targets, fmm.x0, fmm.r0);
  Nodes<real_t> nodes = build_tree(sources, targets, leafs, nonleafs, fmm);
  balance_tree(nodes, sources, targets, leafs, nonleafs, fmm);

  init_rel_coord();
  set_colleagues(nodes);
  build_list(nodes, fmm);

  Node<real_t>* root = nodes.data();
  fmm.P2M(leafs);
  fmm.M2M(root);
  fmm.P2L(nodes);
  fmm.M2P(leafs);
  fmm.P2P(leafs);
  fmm.M2L(nonleafs);
  fmm.L2L(root);
  fmm.L2P(leafs);

  print("number of sources", args.numBodies);
  print("checking leaf's potential...");
#pragma omp parallel for schedule(dynamic)
  for(size_t i=0; i<leafs.size(); ++i) {
    Node<real_t>* leaf = leafs[i];
    assert(args.numBodies == leaf->trg_value[0]);
  }
  print("assertion passed!");
  return 0;
}
