#include "../include/graph/Graph.hpp"
#include "../include/generators/SpecificGenerators.hpp"
#include "../include/metrics/GraphMetrics.hpp"
#include "../include/parsers/GraphParser.hpp"
#include "../include/serializers/GraphVizSerializer.hpp"

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <sstream>
#include <cmath>

using namespace graph;

class CLI {
private:
    std::unique_ptr<Graph<size_t>> currentGraph_;
    std::map<std::string, std::function<void(const std::vector<std::string>&)>> commands_;
    
public:
    CLI() {
        registerCommands();
    }
    
    void run() {
        std::cout << "'help' to see commands\n\n";
        
        std::string line;
        while (true) {
            std::getline(std::cin, line);
            if (line == "exit" || line == "quit") break;
            if (line.empty()) continue;
            std::vector<std::string> tokens = split(line, ' ');
            auto it = commands_.find(tokens[0]);
            if (it != commands_.end()) {
                try {
                    it->second(tokens);
                } catch (const std::exception& e) {
                    std::cout << "Error: " << e.what() << "\n";
                }
            } else {
                std::cout << "Unknown command: " << tokens[0] << "\n";
                std::cout << "Type 'help' for available commands\n";
            }
        }
    }
    
private:
    void registerCommands() {
        commands_["help"] = [this](const auto& args) { showHelp(); };
        commands_["generate"] = [this](const auto& args) { generateGraph(args); };
        commands_["load"] = [this](const auto& args) { loadGraph(args); };
        commands_["metrics"] = [this](const auto& args) { showMetrics(); };
        commands_["visualize"] = [this](const auto& args) { visualize(args); };
        commands_["info"] = [this](const auto& args) { showInfo(); };
        commands_["test"] = [this](const auto& args) { runMetricsTests(); };
        commands_["test_generators"] = [this](const auto& args) { runGeneratorsTests(); };
        commands_["test_serialization"] = [this](const auto& args) { runSerializationTests(); }; 
    }
    
    void showHelp() {
        std::cout << "\nAvailable commands:\n";
        std::cout << "  generate <type> <params>     - Generate a graph\n";
        std::cout << "    Types:\n";
        std::cout << "      complete N\n";
        std::cout << "      cycle N\n";
        std::cout << "      path N\n";
        std::cout << "      star N\n";
        std::cout << "      wheel N\n";
        std::cout << "      random N P (P = edge probability)\n";
        std::cout << "      tree N\n";
        std::cout << "      bipartite N M\n";
        std::cout << "      cubic N (N even, >=4)\n\n";
        
        std::cout << "  load <filename> <format>     - Load graph from file\n";
        std::cout << "    Formats: edgelist, adjmatrix, dimacs, snap\n\n";
        
        std::cout << "  metrics                       - Show all graph metrics\n";
        std::cout << "  info                          - Show basic graph info\n";
        std::cout << "  visualize <file> [tree] [cycle] - Save as GraphViz\n";
        std::cout << "  test                          - Run metrics tests\n";
        std::cout << "  test_generators               - Run generators tests\n";
        std::cout << "  test_serialization            - Run serialization tests\n";
        std::cout << "  exit / quit                   - Exit program\n\n";
    }
    
    void runMetricsTests() {
        std::cout << "Running Metrics Tests" << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        // Тест 1: Полный граф K3
        {
            std::cout << "[TEST 1] Complete graph K3" << std::endl;
            generators::CompleteGraph<size_t> gen(3);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            double density = metrics.density();
            if (std::abs(density - 1.0) > 0.001) {
                std::cout << "  ✗ Density: expected 1.0, got " << density << std::endl;
                ok = false;
            }
            if (metrics.diameter() != 1) {
                std::cout << "  ✗ Diameter: expected 1, got " << metrics.diameter() << std::endl;
                ok = false;
            }
            if (metrics.connectedComponents() != 1) {
                std::cout << "  ✗ Components: expected 1, got " << metrics.connectedComponents() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 2: Путь P4
        {
            std::cout << "[TEST 2] Path graph P4" << std::endl;
            generators::PathGraph<size_t> gen(4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            double expected_density = 3.0 / 6.0;
            double density = metrics.density();
            if (std::abs(density - expected_density) > 0.001) {
                std::cout << "  ✗ Density: expected " << expected_density << ", got " << density << std::endl;
                ok = false;
            }
            if (metrics.diameter() != 3) {
                std::cout << "  ✗ Diameter: expected 3, got " << metrics.diameter() << std::endl;
                ok = false;
            }
            if (metrics.connectedComponents() != 1) {
                std::cout << "  ✗ Components: expected 1, got " << metrics.connectedComponents() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 3: Цикл C4 (двудольный)
        {
            std::cout << "[TEST 3] Cycle graph C4 (bipartite)" << std::endl;
            generators::CycleGraph<size_t> gen(4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (!metrics.isBipartite()) {
                std::cout << "  ✗ Bipartite: expected true, got false" << std::endl;
                ok = false;
            }
            if (metrics.diameter() != 2) {
                std::cout << "  ✗ Diameter: expected 2, got " << metrics.diameter() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 4: Цикл C3 (НЕ двудольный)
        {
            std::cout << "[TEST 4] Cycle graph C3 (non-bipartite)" << std::endl;
            generators::CycleGraph<size_t> gen(3);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (metrics.isBipartite()) {
                std::cout << "  ✗ Bipartite: expected false, got true" << std::endl;
                ok = false;
            }
            if (metrics.diameter() != 1) {
                std::cout << "  ✗ Diameter: expected 1, got " << metrics.diameter() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 5: Звезда S4
        {
            std::cout << "[TEST 5] Star graph S4" << std::endl;
            generators::StarGraph<size_t> gen(4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (metrics.diameter() != 2) {
                std::cout << "  ✗ Diameter: expected 2, got " << metrics.diameter() << std::endl;
                ok = false;
            }
            if (metrics.connectedComponents() != 1) {
                std::cout << "  ✗ Components: expected 1, got " << metrics.connectedComponents() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 6: Колесо W4
        {
            std::cout << "[TEST 6] Wheel graph W4" << std::endl;
            generators::WheelGraph<size_t> gen(4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (g.vertexCount() != 4) {
                std::cout << "  ✗ Vertices: expected 4, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 6) {
                std::cout << "  ✗ Edges: expected 6, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 7: Полный двудольный K2,3
        {
            std::cout << "[TEST 7] Complete bipartite K2,3" << std::endl;
            generators::BipartiteCompleteGraph<size_t> gen(2, 3);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (!metrics.isBipartite()) {
                std::cout << "  ✗ Bipartite: expected true, got false" << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 6) {
                std::cout << "  ✗ Edges: expected 6, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 8: Транзитивность K3
        {
            std::cout << "[TEST 8] Transitivity of complete graph K3" << std::endl;
            generators::CompleteGraph<size_t> gen(3);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            double transitivity = metrics.transitivity();
            
            if (std::abs(transitivity - 1.0/3.0) > 0.001) {
                std::cout << "  ✗ Transitivity: expected 0.333, got " << transitivity << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 9: Мосты в дереве
        {
            std::cout << "[TEST 9] Bridges in tree" << std::endl;
            generators::TreeGraph<size_t> gen(4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            auto bridges = metrics.bridges();
            if (bridges.size() != 3) {
                std::cout << "  ✗ Bridges: expected 3, got " << bridges.size() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 10: Хроматическое число для K3
        {
            std::cout << "[TEST 10] Chromatic number upper bound K3" << std::endl;
            generators::CompleteGraph<size_t> gen(3);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            int chromatic = metrics.chromaticNumberUpperBound();
            if (chromatic != 3) {
                std::cout << "  ✗ Chromatic: expected 3, got " << chromatic << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Итоги
        std::cout << "========================================" << std::endl;
        std::cout << "Metrics Tests Results: " << passed << " passed, " << failed << " failed" << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
    void runGeneratorsTests() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Running Generators Tests" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        // Тест 1: Complete graph generator
        {
            std::cout << "[TEST 1] Complete graph generator K5" << std::endl;
            generators::CompleteGraph<size_t> gen(5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 10) {
                std::cout << "  ✗ Edges: expected 10, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 2: Cycle graph generator
        {
            std::cout << "[TEST 2] Cycle graph generator C5" << std::endl;
            generators::CycleGraph<size_t> gen(5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 5) {
                std::cout << "  ✗ Edges: expected 5, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            // Проверяем, что степень каждой вершины = 2
            for (auto v : g.getVertices()) {
                if (g.degree(v) != 2) {
                    std::cout << "  ✗ Vertex " << v << " degree: expected 2, got " << g.degree(v) << std::endl;
                    ok = false;
                }
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 3: Path graph generator
        {
            std::cout << "[TEST 3] Path graph generator P5" << std::endl;
            generators::PathGraph<size_t> gen(5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 4) {
                std::cout << "  ✗ Edges: expected 4, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 4: Star graph generator
        {
            std::cout << "[TEST 4] Star graph generator S5" << std::endl;
            generators::StarGraph<size_t> gen(5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 4) {
                std::cout << "  ✗ Edges: expected 4, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 5: Wheel graph generator
        {
            std::cout << "[TEST 5] Wheel graph generator W5" << std::endl;
            generators::WheelGraph<size_t> gen(5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            // W5: 4 внешних ребра + 4 спицы = 8 ребер
            if (g.edgeCount() != 8) {
                std::cout << "  ✗ Edges: expected 8, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 6: Tree generator
        {
            std::cout << "[TEST 6] Tree generator T5" << std::endl;
            generators::TreeGraph<size_t> gen(5);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (g.vertexCount() != 5) {
                std::cout << "  ✗ Vertices: expected 5, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            // Дерево должно иметь V-1 ребер
            if (g.edgeCount() != 4) {
                std::cout << "  ✗ Edges: expected 4, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            // Дерево должно быть связным
            if (metrics.connectedComponents() != 1) {
                std::cout << "  ✗ Not connected" << std::endl;
                ok = false;
            }
            // В дереве нет циклов (проверяем транзитивность = 0)
            if (metrics.transitivity() != 0) {
                std::cout << "  ✗ Has cycles (transitivity = " << metrics.transitivity() << ")" << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 7: Complete bipartite generator
        {
            std::cout << "[TEST 7] Complete bipartite generator K2,4" << std::endl;
            generators::BipartiteCompleteGraph<size_t> gen(2, 4);
            auto g = gen.generate();
            metrics::GraphMetrics<size_t> metrics(g);
            
            bool ok = true;
            if (g.vertexCount() != 6) {
                std::cout << "  ✗ Vertices: expected 6, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            if (g.edgeCount() != 8) { // 2*4 = 8
                std::cout << "  ✗ Edges: expected 8, got " << g.edgeCount() << std::endl;
                ok = false;
            }
            if (!metrics.isBipartite()) {
                std::cout << "  ✗ Not bipartite" << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 8: Random graph generator
        {
            std::cout << "[TEST 8] Random graph generator G(10, 0.5)" << std::endl;
            generators::RandomGraph<size_t> gen(10, 0.5);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 10) {
                std::cout << "  ✗ Vertices: expected 10, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            // Случайный граф должен иметь от 0 до 45 ребер
            if (g.edgeCount() > 45) {
                std::cout << "  ✗ Too many edges: " << g.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 9: Random graph generator extreme cases
        {
            std::cout << "[TEST 9] Random graph generator G(5, 0.0)" << std::endl;
            generators::RandomGraph<size_t> gen0(5, 0.0);
            auto g0 = gen0.generate();
            
            bool ok = true;
            if (g0.edgeCount() != 0) {
                std::cout << "  ✗ p=0: expected 0 edges, got " << g0.edgeCount() << std::endl;
                ok = false;
            }
            
            std::cout << "[TEST 9b] Random graph generator G(5, 1.0)" << std::endl;
            generators::RandomGraph<size_t> gen1(5, 1.0);
            auto g1 = gen1.generate();
            if (g1.edgeCount() != 10) {
                std::cout << "  ✗ p=1: expected 10 edges, got " << g1.edgeCount() << std::endl;
                ok = false;
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 10: Cubic graph generator
        {
            std::cout << "[TEST 10] Cubic graph generator (4 vertices)" << std::endl;
            generators::CubicGraph<size_t> gen(4);
            auto g = gen.generate();
            
            bool ok = true;
            if (g.vertexCount() != 4) {
                std::cout << "  ✗ Vertices: expected 4, got " << g.vertexCount() << std::endl;
                ok = false;
            }
            // В кубическом графе степень каждой вершины = 3
            for (auto v : g.getVertices()) {
                if (g.degree(v) != 3) {
                    std::cout << "  ✗ Vertex " << v << " degree: expected 3, got " << g.degree(v) << std::endl;
                    ok = false;
                }
            }
            
            if (ok) {
                std::cout << "  ✓ PASSED" << std::endl;
                passed++;
            } else {
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Итоги
        std::cout << "========================================" << std::endl;
        std::cout << "Generators Tests Results: " << passed << " passed, " << failed << " failed" << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    void runSerializationTests() {
        std::cout << "\n========================================" << std::endl;
        std::cout << "Running Serialization Tests" << std::endl;
        std::cout << "========================================\n" << std::endl;
        
        int passed = 0;
        int failed = 0;
        
        // Тест 1: Базовая сериализация в GraphViz
        {
            std::cout << "[TEST 1] Basic GraphViz serialization" << std::endl;
            generators::PathGraph<size_t> gen(4);
            auto g = gen.generate();
            
            std::string filename = "test_graphviz.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, false, false);
                
                // Проверяем, что файл создан
                std::ifstream file(filename);
                bool fileExists = file.good();
                file.close();
                
                if (fileExists) {
                    std::cout << "  ✓ File created: " << filename << std::endl;
                    passed++;
                    std::remove(filename.c_str());
                } else {
                    std::cout << "  ✗ File not created" << std::endl;
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 2: Проверка содержимого DOT файла
        {
            std::cout << "[TEST 2] DOT file content validation" << std::endl;
            generators::CycleGraph<size_t> gen(3);
            auto g = gen.generate();
            
            std::string filename = "test_content.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, false, false);
                
                std::ifstream file(filename);
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                file.close();
                
                bool hasGraphDecl = content.find("graph G") != std::string::npos;
                bool hasBrace = content.find("{") != std::string::npos;
                bool hasEdges = content.find("--") != std::string::npos;
                
                if (hasGraphDecl && hasBrace && hasEdges) {
                    std::cout << "  ✓ DOT syntax is valid" << std::endl;
                    passed++;
                } else {
                    std::cout << "  ✗ Invalid DOT syntax" << std::endl;
                    failed++;
                }
                
                std::remove(filename.c_str());
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 3: Сериализация с выделением остова (spanning tree)
        {
            std::cout << "[TEST 3] Serialization with spanning tree highlighting" << std::endl;
            generators::CycleGraph<size_t> gen(5);
            auto g = gen.generate();
            
            std::string filename = "test_spanning.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, true, false);
                
                std::ifstream file(filename);
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                file.close();
                
                // Проверяем, что есть атрибуты цвета (остов выделен)
                bool hasColor = content.find("color") != std::string::npos ||
                                content.find("style") != std::string::npos;
                
                if (hasColor) {
                    std::cout << "  ✓ Spanning tree highlighting present" << std::endl;
                    passed++;
                } else {
                    std::cout << "  ✗ No highlighting found" << std::endl;
                    failed++;
                }
                
                std::remove(filename.c_str());
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 4: Сериализация с выделением случайного цикла
        {
            std::cout << "[TEST 4] Serialization with random cycle highlighting" << std::endl;
            generators::CompleteGraph<size_t> gen(5);
            auto g = gen.generate();
            
            std::string filename = "test_cycle.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, false, true);
                
                std::ifstream file(filename);
                std::string content((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
                file.close();
                
                // Проверяем, что есть выделение
                bool hasHighlight = content.find("color") != std::string::npos ||
                                    content.find("style") != std::string::npos;
                
                if (hasHighlight) {
                    std::cout << "  ✓ Random cycle highlighting present" << std::endl;
                    passed++;
                } else {
                    std::cout << "  ✗ No cycle highlighting found" << std::endl;
                    failed++;
                }
                
                std::remove(filename.c_str());
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 5: Проверка, что случайный цикл действительно разный при多次 вызовах
        {
            std::cout << "[TEST 5] Random cycle varies between calls" << std::endl;
            generators::CompleteGraph<size_t> gen(6);
            auto g = gen.generate();
            
            std::vector<std::string> contents;
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                
                for (int i = 0; i < 3; ++i) {
                    std::string filename = "test_random_" + std::to_string(i) + ".dot";
                    serializer.serialize(g, filename, false, true);
                    
                    std::ifstream file(filename);
                    std::string content((std::istreambuf_iterator<char>(file)),
                                        std::istreambuf_iterator<char>());
                    file.close();
                    contents.push_back(content);
                    std::remove(filename.c_str());
                }
                
                // Проверяем, что не все содержимое одинаковое
                bool allSame = (contents[0] == contents[1] && contents[1] == contents[2]);
                
                if (!allSame) {
                    std::cout << "  ✓ Random cycles differ between runs" << std::endl;
                    passed++;
                } else {
                    std::cout << "  ✗ All cycles are identical (not random)" << std::endl;
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 6: Сериализация пустого графа
        {
            std::cout << "[TEST 6] Serialization of empty graph" << std::endl;
            Graph<size_t> g;
            
            std::string filename = "test_empty.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, false, false);
                
                std::ifstream file(filename);
                bool fileExists = file.good();
                file.close();
                
                if (fileExists) {
                    std::cout << "  ✓ Empty graph serialized without errors" << std::endl;
                    passed++;
                    std::remove(filename.c_str());
                } else {
                    std::cout << "  ✗ Failed to serialize empty graph" << std::endl;
                    failed++;
                }
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Тест 7: Сериализация графа с одной вершиной
        {
            std::cout << "[TEST 7] Serialization of single vertex graph" << std::endl;
            Graph<size_t> g;
            g.addVertex(0);
            
            std::string filename = "test_single.dot";
            
            try {
                serializers::GraphVizSerializer<size_t> serializer;
                serializer.serialize(g, filename, false, false);
            
            std::ifstream file(filename);
            std::string content((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
            file.close();
            
            bool hasVertex = content.find("0") != std::string::npos;
            
            if (hasVertex) {
                std::cout << "  ✓ Single vertex graph serialized correctly" << std::endl;
                passed++;
            } else {
                std::cout << "  ✗ Vertex missing in output" << std::endl;
                failed++;
            }
            
            std::remove(filename.c_str());
            } catch (const std::exception& e) {
                std::cout << "  ✗ Exception: " << e.what() << std::endl;
                failed++;
            }
            std::cout << std::endl;
        }
        
        // Итоги
        std::cout << "========================================" << std::endl;
        std::cout << "Serialization Tests Results: " << passed << " passed, " << failed << " failed" << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    void generateGraph(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << "Usage: generate <type> <params>\n";
            return;
        }
        
        std::string type = args[1];
        
        try {
            if (type == "complete" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::CompleteGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated complete graph K" << n << "\n";
            }
            else if (type == "cycle" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::CycleGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated cycle graph C" << n << "\n";
            }
            else if (type == "path" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::PathGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated path graph P" << n << "\n";
            }
            else if (type == "star" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::StarGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated star graph S" << n << "\n";
            }
            else if (type == "wheel" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::WheelGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated wheel graph W" << n << "\n";
            }
            else if (type == "random" && args.size() >= 4) {
                size_t n = std::stoul(args[2]);
                double p = std::stod(args[3]);
                generators::RandomGraph<size_t> gen(n, p);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated random graph G(" << n << ", " << p << ")\n";
            }
            else if (type == "tree" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::TreeGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated tree T" << n << "\n";
            }
            else if (type == "bipartite" && args.size() >= 4) {
                size_t n = std::stoul(args[2]);
                size_t m = std::stoul(args[3]);
                generators::BipartiteCompleteGraph<size_t> gen(n, m);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated complete bipartite graph K" << n << "," << m << "\n";
            }
            else if (type == "cubic" && args.size() >= 3) {
                size_t n = std::stoul(args[2]);
                generators::CubicGraph<size_t> gen(n);
                currentGraph_ = std::make_unique<Graph<size_t>>(gen.generate());
                std::cout << "Generated cubic graph on " << n << " vertices\n";
            }
            else {
                std::cout << "Unknown graph type or missing parameters\n";
            }
        } catch (const std::exception& e) {
            std::cout << "Generation failed: " << e.what() << "\n";
        }
    }
    
    void loadGraph(const std::vector<std::string>& args) {
        if (args.size() < 3) {
            std::cout << "Usage: load <filename> <format>\n";
            return;
        }
        
        std::string filename = args[1];
        std::string format = args[2];
        
        std::unique_ptr<parsers::GraphParser<size_t>> parser;
        
        if (format == "edgelist") {
            parser = std::make_unique<parsers::EdgeListParser<size_t>>();
        } else if (format == "adjmatrix") {
            parser = std::make_unique<parsers::AdjMatrixParser<size_t>>();
        } else if (format == "dimacs") {
            parser = std::make_unique<parsers::DIMACSParser<size_t>>();
        } else if (format == "snap") {
            parser = std::make_unique<parsers::SNAPParser<size_t>>();
        } else {
            std::cout << "Unknown format. Supported: edgelist, adjmatrix, dimacs, snap\n";
            return;
        }
        
        try {
            currentGraph_ = std::make_unique<Graph<size_t>>(parser->parse(filename));
            std::cout << "Graph loaded from " << filename << "\n";
            std::cout << "Vertices: " << currentGraph_->vertexCount() << "\n";
            std::cout << "Edges: " << currentGraph_->edgeCount() << "\n";
        } catch (const std::exception& e) {
            std::cout << "Failed to load graph: " << e.what() << "\n";
        }
    }
    
    void showMetrics() {
        if (!currentGraph_) {
            std::cout << "No graph loaded. Use 'generate' or 'load' first.\n";
            return;
        }
        
        metrics::GraphMetrics<size_t> metrics(*currentGraph_);
        
        std::cout << "\n========================================" << std::endl;
        std::cout << "Graph Metrics" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "Vertices (вершины):                 " << currentGraph_->vertexCount() << std::endl;
        std::cout << "Edges (ребра):                      " << currentGraph_->edgeCount() << std::endl;
        std::cout << "Density (плотность):                " << metrics.density() << std::endl;
        std::cout << "Diameter (диаметр):                 " << metrics.diameter() << std::endl;
        std::cout << "Connected components (комп. свзян.):" << metrics.connectedComponents() << std::endl;
        std::cout << "Articulation points (т сочленения): " << metrics.articulationPoints().size() << std::endl;
        std::cout << "Bridges (мосты):                    " << metrics.bridges().size() << std::endl;
        std::cout << "Bipartite (двудольный):             " << (metrics.isBipartite() ? "Yes" : "No") << std::endl;
        std::cout << "Chromatic number (хром. число):     " << metrics.chromaticNumberUpperBound() << std::endl;
        std::cout << "Transitivity (транзитивность):      " << metrics.transitivity() << std::endl;
        std::cout << "========================================\n" << std::endl;
    }
    
    void visualize(const std::vector<std::string>& args) {
        if (!currentGraph_) {
            std::cout << "No graph loaded. Use 'generate' or 'load' first.\n";
            return;
        }
        
        if (args.size() < 2) {
            std::cout << "Usage: visualize <filename> [tree] [cycle]\n";
            return;
        }
        
        std::string filename = args[1];
        bool showTree = false;
        bool showCycle = false;
        
        for (size_t i = 2; i < args.size(); ++i) {
            if (args[i] == "tree") showTree = true;
            if (args[i] == "cycle") showCycle = true;
        }
        
        try {
            serializers::GraphVizSerializer<size_t> serializer;
            serializer.serialize(*currentGraph_, filename, showTree, showCycle);
            
            std::cout << "Graph saved to " << filename << "\n";
            std::cout << "Render with: dot -Tpng " << filename << " -o output.png\n";
        } catch (const std::exception& e) {
            std::cout << "Failed to save: " << e.what() << "\n";
        }
    }
    
    void showInfo() {
        if (!currentGraph_) {
            std::cout << "No graph loaded. Use 'generate' or 'load' first.\n";
            return;
        }
        
        std::cout << "\nGraph Information:\n";
        std::cout << "  Vertices: " << currentGraph_->vertexCount() << "\n";
        std::cout << "  Edges: " << currentGraph_->edgeCount() << "\n";
        std::cout << "  Directed: " << (currentGraph_->isDirected() ? "Yes" : "No") << "\n";
        
        std::cout << "  Degree sequence: ";
        std::vector<int> degrees;
        for (auto v : currentGraph_->getVertices()) {
            degrees.push_back(currentGraph_->degree(v));
        }
        std::sort(degrees.begin(), degrees.end(), std::greater<int>());
        for (size_t i = 0; i < std::min(degrees.size(), size_t(10)); ++i) {
            std::cout << degrees[i] << " ";
        }
        if (degrees.size() > 10) std::cout << "...";
        std::cout << "\n\n";
    }
    
    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;
        while (std::getline(ss, item, delim)) {
            if (!item.empty()) result.push_back(item);
        }
        return result;
    }
};

int main() {
    CLI cli;
    cli.run();
    return 0;
}