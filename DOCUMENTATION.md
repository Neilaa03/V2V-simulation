# V2V Communication Simulator - Technical Documentation

## Description

This project implements a Vehicle-to-Vehicle (V2V) communication simulator on an OpenStreetMap. The application enables real-time visualization of vehicle movement on a road network and analyzes their wireless communication capabilities based on transmission range.

The simulator uses real cartographic data (OSM PBF format) to build a road graph on which vehicles move autonomously. An interference graph models possible connections between vehicles, with support for transitive closure to represent multi-hop communications.

---

## Table of Contents

1. [Features](#features)
2. [Project Architecture](#project-architecture)
3. [Dependencies](#dependencies)
4. [Building](#building)
5. [Usage](#usage)
6. [File Structure](#file-structure)
7. [Module Description](#module-description)
8. [Implemented Algorithms](#implemented-algorithms)
9. [User Interface](#user-interface)
10. [Keyboard Controls](#keyboard-controls)

---

## Features

- Load and parse OpenStreetMap files (PBF format)
- Automatic road graph construction from OSM data
- Real-time simulation of 1 to 3000 vehicles
- Interference graph to model V2V communications
- Transitive closure for multi-hop communications
- Spatial optimization with hierarchical grid (K-means algorithm)
- Cartographic visualization with XYZ tiles (dark and light themes)
- Modern user interface with control panels
- Real-time statistics (connections, comparisons, performance)

---

## Project Architecture

```
projet_v2v/
├── include/                    # Header files (.h)
│   ├── graph_types.h          # Boost Graph type definitions
│   ├── osm_reader.h           # OSM file reader
│   ├── graph_builder.h        # Road graph builder
│   ├── vehicule.h             # Vehicle class
│   ├── simulator.h            # Simulation engine
│   ├── interference_graph.h   # V2V interference graph
│   ├── spatial_grid.h         # Optimized spatial grid
│   ├── map_view.h             # Map visualization widget
│   ├── overlay_ui.h           # Overlay user interface
│   └── vehicle_renderer.h     # Vehicle SVG rendering
├── src/                        # Source files (.cpp)
│   ├── main.cpp               # Application entry point
│   ├── osm_reader.cpp         # OSM reader implementation
│   ├── graph_builder.cpp      # Graph builder implementation
│   ├── vehicule.cpp           # Vehicle movement logic
│   ├── simulator.cpp          # Simulation loop
│   ├── interference_graph.cpp # Interference calculation
│   ├── spatial_grid.cpp       # K-means algorithm and grid
│   ├── map_view.cpp           # Map and vehicle rendering
│   ├── overlay_ui.cpp         # Qt UI components
│   └── vehicle_renderer.cpp   # Vector vehicle rendering
├── data/                       # Data
│   └── strasbourg.osm.pbf     # OpenStreetMap map
├── CMakeLists.txt             # CMake configuration
└── README.md                  # This file
```

---

## Dependencies

### Required Libraries

| Library | Version | Usage |
|---------|---------|-------|
| Qt5 or Qt6 | 5.15+ / 6.x | GUI, networking, SVG |
| Boost | 1.71+ | Graph structures (adjacency_list) |
| libosmium | 2.15+ | OSM PBF file parsing |
| PROJ | 6.0+ | Geographic projections |
| zlib | - | Compression (osmium dependency) |
| bz2 | - | Compression (osmium dependency) |
| expat | - | XML parsing (osmium dependency) |

### Dependency Installation (Ubuntu/Debian)

```bash
sudo apt update
sudo apt install -y \
    build-essential cmake \
    qtbase5-dev qttools5-dev libqt5svg5-dev \
    libboost-all-dev \
    libosmium2-dev \
    libproj-dev \
    zlib1g-dev libbz2-dev libexpat1-dev
```

---

## Building

### Configuration and compilation

```bash
# Create build directory
mkdir -p build && cd build

# Configure with CMake
cmake ..

# Compile
make -j$(nproc)

# Or with Ninja (faster)
cmake -G Ninja ..
ninja
```

### Execution

```bash
./ConnectedVehicles
```

Note: The executable must be able to access the `data/strasbourg.osm.pbf` file (relative path `../../data/` from the build directory).

---

## Usage

On startup, the application:

1. Loads OSM data for Strasbourg
2. Builds the road graph (vertices = intersections, edges = roads)
3. Generates 2000 randomly positioned vehicles
4. Starts the simulation at 20 FPS

The interface allows you to:
- Navigate the map (drag-and-drop, scroll wheel to zoom)
- Adjust the number of vehicles (1 to 3000)
- Configure antennas for spatial optimization
- Visualize connections between vehicles
- Display transmission radii
- Enable/disable transitive connections

---

## File Structure

### Header files (include/)

#### graph_types.h
Defines the fundamental types of the road graph based on Boost Graph Library:
- `VertexData`: vertex data (OSM id, latitude, longitude)
- `EdgeData`: edge data (distance, one-way, road type)
- `RoadGraph`: alias for `boost::adjacency_list` configured for the road network

#### osm_reader.h
`OSMReader` class to parse OSM PBF files:
- `OSMNode` and `OSMWay` structures to store raw data
- `read()` method using libosmium for parsing

#### graph_builder.h
`GraphBuilder` class to build the Boost graph:
- OSM data conversion to graph
- Geodesic distance calculation (Haversine formula)
- One-way road handling

#### vehicule.h
`Vehicule` class representing a vehicle in the simulation:
- Position on a graph edge
- Navigation towards a goal
- Transmission range management
- Direction (heading) calculation for rendering

#### simulator.h
`Simulator` class (QObject) orchestrating the simulation:
- Lifecycle management (start, pause, stop)
- Vehicle updates at each tick
- Interference graph reconstruction
- Dynamic vehicle fleet management

#### interference_graph.h
`InterferenceGraph` class modeling V2V communications:
- Adjacency list for direct connections
- Transitive closure via BFS
- Spatial grid optimization

#### spatial_grid.h
`SpatialGrid` class for distance calculation optimization:
- `MacroAntenna` and `MicroAntenna` structures
- K-means algorithm for antenna placement
- Complexity reduction from O(n²) to O(n)

#### map_view.h
`MapView` class (QWidget) for map rendering:
- XYZ tile display (OpenStreetMap, CartoDB)
- Tile cache per theme
- Vehicle and connection rendering
- User interaction handling

#### overlay_ui.h
User interface classes:
- `TopBar`: top bar with main controls
- `ParametersPanel`: sliders and configuration toggles
- `StatsPanel`: real-time statistics
- `BottomMenu`: panel container with animation
- `ZoomControls`: zoom buttons
- `UIOverlay`: interface orchestrator

---

## Module Description

### OSM Module (osm_reader, graph_builder)

The OSM module is responsible for loading cartographic data:

1. **OSMReader** parses the PBF file using libosmium
   - Extracts nodes (GPS coordinates)
   - Extracts ways (sequences of nodes forming roads)
   - Filters by road type (highway=*)

2. **GraphBuilder** builds the road graph
   - Creates a Boost vertex for each referenced node
   - Creates an edge for each road segment
   - Calculates distance in meters via Haversine formula

### Simulation Module (simulator, vehicule)

The simulation engine manages vehicle movement:

1. **Simulator** uses a QTimer for regular ticks (50ms)
   - Calls `update()` on each vehicle
   - Rebuilds the interference graph
   - Emits `ticked()` signal for rendering

2. **Vehicule** moves along graph edges
   - Linear interpolation on current edge
   - Random selection of next edge at intersection
   - Direction reversal when goal is reached

### Communication Module (interference_graph, spatial_grid)

The interference graph models communication capabilities:

1. **Graph construction** (at each tick)
   - Two vehicles are neighbors if their distance < transmission range
   - Without optimization: O(n²) comparisons
   - With spatial grid: O(n) average comparisons

2. **Transitive closure** (optional)
   - BFS from each vehicle to find all reachable vehicles
   - Allows modeling multi-hop communications

3. **Hierarchical spatial grid**
   - Level 1: Macro-antennas placed by K-means
   - Level 2: Micro-antennas subdivided in each macro-zone
   - Only vehicles in neighboring zones are compared

### Rendering Module (map_view, vehicle_renderer, overlay_ui)

Rendering uses Qt's painting system:

1. **MapView** draws the map and entities
   - XYZ tiles loaded from network or cache
   - Web Mercator projection (EPSG:3857)
   - Adaptive rendering based on zoom level

2. **VehicleRenderer** draws vehicles
   - SVG mode for zoom >= 13 (high quality)
   - Point mode for zoom < 13 (performance)
   - Consistent coloring based on ID

3. **UIOverlay** overlays the user interface
   - Semi-transparent background with blur
   - Smooth panel animation

---

## Implemented Algorithms

### K-means Algorithm for Antenna Placement

```
Input: List of vehicles V, number of antennas K
Output: Optimal positions of K antennas

1. Initialize K centers randomly among vehicle positions
2. Repeat until convergence (max 50 iterations):
   a. Assign each vehicle to the nearest center
   b. Recalculate each center as the centroid of its vehicles
   c. If no center moved, terminate
3. Return K centers as antenna positions
```

Complexity: O(n * K * iterations)

### BFS Algorithm for Transitive Closure

```
Input: Adjacency graph G, source vertex s
Output: Set of vertices reachable from s

1. Create a queue Q and visited set
2. Enqueue s, mark s as visited
3. While Q is not empty:
   a. Dequeue a vertex u
   b. For each neighbor v of u:
      - If v not visited: mark visited, enqueue v
4. Return visited set
```

Complexity: O(V + E) per source vertex

### Spatial Grid Optimization

```
Input: Vehicle v, grid G
Output: Candidate vehicles for distance comparison

1. Find the micro-antenna M containing v
2. Get neighboring micro-antennas of M
3. Collect all vehicles in M and its neighbors
4. Return this list (instead of all vehicles)
```

Reduction: from O(n) to O(n/k) average comparisons, where k is the number of micro-antennas.

---

## User Interface

### TopBar (Top Bar)

| Element | Description |
|---------|-------------|
| Logo + Title | Application identifier |
| Status Badge | Indicates if simulation is running or paused |
| Map Info | Zoom level and center coordinates |
| Theme Button | Toggle between dark and light theme |
| Quality Button | Toggle between fast and high quality mode |
| Play/Pause Button | Simulation control |

### ParametersPanel (Parameters Panel)

| Parameter | Range | Description |
|-----------|-------|-------------|
| Number of vehicles | 1 - 3000 | Dynamically adjust the fleet |
| Large antennas | 0 - 50 | Macro-cells for K-means |
| Small antennas | 0 - 200 | Micro-cells per macro |
| Transmission radius | 10 - 1000 m | Communication range |
| Show connections | On/Off | Lines between connected vehicles |
| Show radii | On/Off | Transmission range circles |
| Transitive connections | On/Off | Enable multi-hop calculation |

### StatsPanel (Statistics Panel)

| Statistic | Description |
|-----------|-------------|
| Active vehicles | Total number of vehicles |
| Connected vehicles | Vehicles with at least one neighbor |
| Total connections | Number of edges in the graph |
| Connection rate | Percentage of connected vehicles |
| Comparisons/tick | Number of distance calculations per tick |
| Avg neighbors/vehicle | Average graph degree |
| Calculation time | Duration of graph construction |

---

## Keyboard Controls

| Key | Action |
|-----|--------|
| Arrow keys | Pan view |
| +/- | Zoom in/out |
| Space | Play/Pause simulation |
| T | Toggle transitive connections |
| B | Toggle dark/light theme |
| L | Toggle low/high quality mode |
