#include <vector>
#include <fstream>
#include <unordered_set>
#include <map>

#include <boost/foreach.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>

using namespace boost;

int main (int argc, char* argv[]) {
  try {
    // check arguments, expect a filename
    if (argc != 6) {
      std::cerr << "Usage: " << argv[0] << " nodes.tsv edges.tsv start inc max\n";
      exit(1);
    }

    typedef long long node_id_t;
    int i = 0;

    struct Node {
      node_id_t id;
    };

    struct Segment {
      int length;
    };

    typedef adjacency_list<
      vecS,
      vecS,
      undirectedS,
      Node,
      Segment
    > Graph;

    Graph graph;

    typedef graph_traits<Graph>::vertex_descriptor Vertex;
    typedef graph_traits<Graph>::edge_descriptor Edge;
    typedef graph_traits<Graph>::vertices_size_type VertexIndex;
    typedef graph_traits<Graph>::out_edge_iterator out_edge_iterator;

    // map from node_id -> vertex (to find vertex by id)
    std::map<node_id_t, Vertex> node_ids;

    std::cerr << "graph init" << std::endl;


    std::ifstream nodes_file(argv[1]);
    for (std::string line; std::getline(nodes_file, line); ) {
      std::vector<std::string>  line_data;
      std::stringstream   lineStream(line);
      std::string         cell;

      line_data.clear();
      while(std::getline(lineStream, cell, '\t')) {
          line_data.push_back(cell);
      }

      node_id_t node_id = std::stoll(line_data[0]);
      Vertex v = add_vertex(graph);

      node_ids[node_id] = v;
      graph[v].id = node_id;

      ++i;
    }
    std::cerr << i << " nodes loaded" << std::endl;


    int edges_added = 0;

    std::vector<VertexIndex> rank(num_vertices(graph));
    std::vector<Vertex> parent(num_vertices(graph));

    typedef VertexIndex* Rank;
    typedef Vertex* Parent;

    disjoint_sets<Rank, Parent> ds(&rank[0], &parent[0]);

    initialize_incremental_components(graph, ds);
    incremental_components(graph, ds);
    std::cerr << "icc init" << std::endl;

    Edge edge;
    bool flag;

    std::ifstream edges_file(argv[2]);

    int d = std::stoi(argv[3]);
    const int step = std::stoi(argv[4]);
    const int max = std::stoi(argv[5]);

    std::ofstream nodes_output_file;
    std::ofstream edges_output_file;
    std::ofstream component_size_file;
    std::ofstream num_components_file;
    char buf[50];

    for (std::string line; std::getline(edges_file, line); ) {
      std::vector<std::string>  line_data;
      std::stringstream   lineStream(line);
      std::string         cell;

      line_data.clear();
      while(std::getline(lineStream, cell, '\t')) {
          line_data.push_back(cell);
      }

      long long ida = std::stoll(line_data[0]);
      long long idb = std::stoll(line_data[1]);
      int length = std::stoi(line_data[2]);

      if(length > d){

        std::sprintf(buf,"output/percolation_nodes-%04d.txt", d);
        nodes_output_file.open(buf);

        std::sprintf(buf,"output/percolation_edges-%04d.txt", d);
        edges_output_file.open(buf);

        std::sprintf(buf,"output/component_size-%04d.txt", d);
        component_size_file.open(buf);

        std::sprintf(buf,"output/num_components-%04d.txt", d);
        num_components_file.open(buf);

        std::map<Vertex,int> component_sizes;
        std::unordered_set<Vertex> cnode_ids_dup;

        // iterate through all nodes
        BOOST_FOREACH(Vertex vertex, vertices(graph)) {
          Vertex lead = ds.find_set(vertex);
          component_sizes[lead]++;
        }

        BOOST_FOREACH(Vertex vertex, vertices(graph)) {
          Vertex lead = ds.find_set(vertex);
          int csize = component_sizes[lead];

          if(csize > 50){
            node_id_t lead_id = graph[lead].id;
            node_id_t cid = graph[vertex].id;
            cnode_ids_dup.insert(vertex);
            nodes_output_file << lead_id << "\t" << cid << std::endl;

            std::pair <out_edge_iterator, out_edge_iterator> it_range = out_edges(node_ids[cid], graph);

            // consider each edge associated with that id
            for (out_edge_iterator it = it_range.first; it!=it_range.second; ++it){
              Edge edge = *it;
              Vertex target_v = target(edge, graph);

              // if it's NOT going to a node we've already considered, print it
              auto search_c = cnode_ids_dup.find(target_v);
              if(search_c == cnode_ids_dup.end()){
                int e_length = graph[edge].length;
                node_id_t target_id = graph[target_v].id;
                edges_output_file << lead_id << "\t" << cid << "\t" << target_id << "\t" << e_length << std::endl;
              }
            }
          }
        }

        int num_components = 0;
        for (const auto &pair : component_sizes) {
          component_size_file << pair.first << "\t" << pair.second << std::endl;
          num_components++;
        }

        num_components_file << num_components << std::endl;

        nodes_output_file.close();
        edges_output_file.close();
        component_size_file.close();
        num_components_file.close();
        std::cerr << d << std::endl;

        if (d > max) {
          break;
        }
        while(length > d){
          d += step;
        }
      }

      // add edge
      boost::tie(edge, flag) = add_edge(node_ids[ida], node_ids[idb], graph);
      // store edge length
      graph[edge].length = length;
      // connect components by edge
      ds.union_set(node_ids[ida], node_ids[idb]);

      ++edges_added;

      if(edges_added % 10000000 == 0){
        std::cerr << "added " << edges_added/1000000 << " million   edges" << std::endl;
      }

    }


  } catch(const std::runtime_error& e) {

    fprintf(stderr, "Failed due to runtime error: %s\n", e.what());
    exit(EXIT_FAILURE);

  }

  return(0);
}
