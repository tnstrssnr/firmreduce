#include <firm.h>
#include <pass_utils.h>

#include <stdlib.h>
#include <time.h>

int is_Member_(const ir_node* node) {
    if(!is_Member(node)) return 0;

    ir_graph* irg = get_irn_irg(node);
    edges_activate(irg);

    int has_no_out_edge_member = 1;
    foreach_out_edge(node, edge) {
            if(is_Member(get_edge_src_irn(edge))) {
               has_no_out_edge_member = 0;
            }
        }
    edges_deactivate(irg);
    return has_no_out_edge_member;
}

void replace_member(ir_node* node) {

    // TODO: ?

}

int pass_replace_members_individual(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Member_;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    srand(time(NULL));
    ir_node* node = container->nodes[rand() % container->nodes_n];
    replace_member(node);
    
    collect_nodes(irg, container);
    return 1;
}

int pass_replace_members(ir_graph* irg, void* data) {
    
    ir_node_container* container = malloc(sizeof(ir_node_container));
    container->func = is_Member_;
    collect_nodes(irg, container);

    if (container->nodes_n == 0) {
        free(container);
        return 0;
    }
    
    for(int i = 0; i < container->nodes_n; i++) {   
        replace_member(container->nodes[i]);
    }
    
    
    free(container);
    return 1;
}

int main(int argc, char** argv) {
    return 0;
}