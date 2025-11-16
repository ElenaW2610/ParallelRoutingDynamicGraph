class DynamicGraph {
    private:
        std::vector<std::vector<Vertex>> adj_;
        std::vector<std::vector<Vertex>> snapshot_adj_;
        std::vector<bool> active_;

    public:
        using Vertex = int;
    
        explicit DynamicGraph(std::size_t num_vertices);
    
        void add_edge(Vertex u, Vertex v);
        void remove_edge(Vertex u, Vertex v);
        void add_vertex(Vertex v);
        void remove_vertex(Vertex v);
        const std::vector<Vertex>& neighbors(Vertex v) const;
    
        void snapshot();
    
        std::vector<std::vector<Vertex>>
        min_cost_routing(const std::vector<std::pair<Vertex, Vertex>>& pairs, int max_depth = 5) const;
    
        std::vector<int> k_cores() const;
    };
    