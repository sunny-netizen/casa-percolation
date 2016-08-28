#include <vector>
#include <fstream>
#include <map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/labeled_graph.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace boost;

int main (int argc, char* argv[]) {
  try {
    // check arguments, expect a filename
    if (argc != 3) {
      std::cerr << "Usage: " << argv[0] << " edges.tsv distance\n";
      exit(1);
    }

    // edges file
    std::ifstream edges_file(argv[1]);
    // distance threshold
    int d = std::stoi(argv[2]);

    typedef long long node_id_t;

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

    typedef graph_traits<Graph>::vertex_descriptor Vertex;
    typedef graph_traits<Graph>::edge_descriptor Edge;

    Graph graph;
    // map from node_id -> vertex (to find vertex by id, when checking if added)
    std::map<node_id_t, Vertex> node_ids;

    for (std::string line; std::getline(edges_file, line); ) {
      bool flag;
      Edge edge;
      Vertex v;

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
        break;
      }

      auto search_a = node_ids.find(ida);
      if(search_a == node_ids.end()) {
        v = add_vertex(graph);
        node_ids[ida] = v;
        graph[v].id = ida;
      }

      auto search_b = node_ids.find(idb);
      if(search_b == node_ids.end()) {
        v = add_vertex(graph);
        node_ids[idb] = v;
        graph[v].id = idb;
      }

      // add edge
      boost::tie(edge, flag) = add_edge(node_ids[ida], node_ids[idb], graph);
      // store edge length
      graph[edge].length = length;

    }
    std::cerr << "edges loaded" << std::endl;

    // compute connected components
    std::vector<int> component_map(num_vertices(graph));
    int num_components = connected_components(graph, &component_map[0]);
    std::map<int,int> component_sizes;

    char buf[50];

    std::ofstream nodes_output_file;
    std::sprintf(buf,"output/percolation_nodes-%04d.txt", d);
    nodes_output_file.open(buf);

    std::ofstream component_size_file;
    std::sprintf(buf,"output/component_size-%04d.txt", d);
    component_size_file.open(buf);

    std::vector<int>::size_type i;
    for (i = 0; i != component_map.size(); ++i){
      int component_number = component_map[i];
      component_sizes[component_number]++;
      node_id_t cid = graph[i].id;

      nodes_output_file << component_number << "\t" << cid << std::endl;
    }

    for (const auto &pair : component_sizes) {
      component_size_file << pair.first << "\t" << pair.second << std::endl;
    }

    nodes_output_file.close();
    component_size_file.close();

  } catch(const std::runtime_error& e) {
    fprintf(stderr, "Failed due to runtime error: %s\n", e.what());
    exit(EXIT_FAILURE);
  }

  return(0);
}
