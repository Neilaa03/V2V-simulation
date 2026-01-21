#include "graph_builder.h"
#include <iostream>
#include <cmath>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>

using namespace std;

// Constructeur : initialise les références vers les données OSM
GraphBuilder::GraphBuilder(const vector<OSMNode>& n, const vector<OSMWay>& w)
    : nodes(n), ways(w) {}

// Construit le graphe à partir des données OSM
void GraphBuilder::buildGraph() {

    // ==============================
    // Étape 1 : création des sommets
    // ==============================
    for (const auto& n : nodes) {
        Vertex v = boost::add_vertex(graph);
        graph[v].id  = n.id;
        graph[v].lat = n.lat;
        graph[v].lon = n.lon;
        idToVertex[n.id] = v;
    }

    // ==============================
    // Étape 2 : création des arêtes
    // ==============================
    for (const auto& way : ways) {

        for (size_t i = 1; i < way.nodeRefs.size(); ++i) {

            long id1 = way.nodeRefs[i - 1];
            long id2 = way.nodeRefs[i];

            // Vérifie que les deux nœuds existent
            if (!idToVertex.count(id1) || !idToVertex.count(id2))
                continue;

            Vertex v1 = idToVertex[id1];
            Vertex v2 = idToVertex[id2];

            double dist = distance(
                graph[v1].lat, graph[v1].lon,
                graph[v2].lat, graph[v2].lon
                );

            // ==============================
            // Gestion du sens de circulation
            // ==============================

            if (way.oneway) {
                // Sens unique : v1 -> v2
                Edge e; bool inserted;
                tie(e, inserted) = boost::add_edge(v1, v2, graph);
                if (inserted) {
                    graph[e].distance = dist;
                    graph[e].oneway   = true;
                    graph[e].type     = way.highwayType;
                }

            } else {
                // Double sens : v1 -> v2
                Edge e1; bool i1;
                tie(e1, i1) = boost::add_edge(v1, v2, graph);
                if (i1) {
                    graph[e1].distance = dist;
                    graph[e1].oneway   = false;
                    graph[e1].type     = way.highwayType;
                }

                // Double sens : v2 -> v1
                Edge e2; bool i2;
                tie(e2, i2) = boost::add_edge(v2, v1, graph);
                if (i2) {
                    graph[e2].distance = dist;
                    graph[e2].oneway   = false;
                    graph[e2].type     = way.highwayType;
                }
            }
        }
    }

    cout << "Graphe construit avec succès (sens de circulation respecté)." << endl;
}

// ==========================================================
// Distance géographique (optimisée)
// ==========================================================
double GraphBuilder::distance(double lat1, double lon1,
                              double lat2, double lon2) {

    const double R = 6371000.0; // Rayon de la Terre (m)

    double dLat = (lat2 - lat1);
    double dLon = (lon2 - lon1);

    // Approximation rapide pour petites distances
    if (abs(dLat) < 0.02 && abs(dLon) < 0.02) {
        double x = dLon * M_PI / 180.0 *
                   cos((lat1 + lat2) / 2.0 * M_PI / 180.0);
        double y = dLat * M_PI / 180.0;
        return R * sqrt(x * x + y * y);
    }

    // Haversine complète
    dLat *= M_PI / 180.0;
    dLon *= M_PI / 180.0;

    double a = sin(dLat / 2) * sin(dLat / 2) +
               cos(lat1 * M_PI / 180.0) *
                   cos(lat2 * M_PI / 180.0) *
                   sin(dLon / 2) * sin(dLon / 2);

    double c = 2 * atan2(sqrt(a), sqrt(1 - a));
    return R * c;
}

// ==========================================================
// Résumé du graphe
// ==========================================================
void GraphBuilder::printSummary() const {
    cout << "Résumé du graphe :" << endl;
    cout << "  Sommets : " << boost::num_vertices(graph) << endl;
    cout << "  Arêtes  : " << boost::num_edges(graph) << endl;
}
