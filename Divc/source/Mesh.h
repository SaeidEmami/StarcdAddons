/*
* File:   Mesh.h
* Author: Saeid Emami
*
* Created on June 3, 2013, 5:58 PM
*/

#ifndef MESH_H
#define MESH_H

#include <vector>
#include <set>
#include <list>
#include <map>
#include <memory>
#include <exception>
#include <string>
#include <iomanip>
#include <sstream>
#include <ostream>
#include <unordered_set>
#include <unordered_map>
#include <thread>
#include <mutex>

struct TCell {
    std::vector<size_t> vertices;
    size_t type;

    TCell(): type(0) {}
    TCell(const std::vector<size_t>& _v, size_t _type): vertices(_v), type(_type) {}
};

struct TVertex {
    double x, y, z;
    size_t type;

    TVertex() : x(0), y(0), z(0), type(0) {}
    TVertex(const double _x, const double _y, const double _z, const size_t _type): x(_x), y(_y), z(_z), type(_type) {}

    bool operator== (const TVertex& v) const {return (x == v.x && y == v.y && z == v.z);};
    bool operator!= (const TVertex& v) const {return (x != v.x || y != v.y || z != v.z);};
    bool operator< (const TVertex& v) const {
        if (x < v.x)
            return true;
        else if (x == v.x) {
            if (y < v.y)
                return true;
            else if (y == v.y)
                return (z < v.z);
            else
                return false;
        }
        else
            return false;
    }
};

enum cellANSYS_t{undefined_ANSYS = -1, SOLID45 = 45, SHELL63 = 63};
enum cellABAQUS_t{undefined_ABAQUS = -1, C3D8, C3D8R};

struct TCellType {
    short dimension; // -1: undefined, 0: point, 1: line, 2: shell, 3: solid
    unsigned long long mType;
    unsigned long long realConstant;
    unsigned long long cellCSys;
    cellANSYS_t ansysType;
    cellABAQUS_t abaqusType;
    std::string name;

    TCellType(): dimension(-1), mType(0), realConstant(0), cellCSys(0), 
        ansysType(undefined_ANSYS), abaqusType(undefined_ABAQUS) {}
};

struct TVertexType {
    std::string name;

    TVertexType() {
    }
};

struct TEdge {
    size_t V1, V2;
    TEdge() {};
    TEdge(const size_t v1, const size_t v2);
    TEdge(const TEdge& t);
    TEdge& operator=(const TEdge& t);
    void assignVertices(const size_t v1, const size_t v2);
    bool operator== (const TEdge& t) const;
    bool operator!= (const TEdge& t) const;
    bool operator< (const TEdge& t) const;
    std::string str() const;
    struct hasher {
        size_t operator () (const TEdge& anEdge) const {return anEdge.V1;}
    };
};

struct TSurface {
    std::vector<size_t> V;
    TSurface() {};
    TSurface(const std::vector<size_t>& v);
    TSurface(const TSurface& s);
    TSurface& operator=(const TSurface& s);
    void assignVertices(const std::vector<size_t>& v);
    bool operator== (const TSurface& s) const;
    bool operator!= (const TSurface& s) const;
    bool operator< (const TSurface& s) const;
    std::string str() const;
    struct hasher {
        size_t operator () (const TSurface& aSurf) const
        {
            if (aSurf.V.size() == 0) return 0;
            return aSurf.V[0];
        }
    };
    void flip();
};

struct TModel {
    std::vector<std::unique_ptr<TVertex>> global_vertices;
    void v(const size_t vertexNumber, const double x, const double y, const double z, const size_t type);
    void vdelAll();
    void vdel(const size_t vertexNumber);
    template <typename T> void vlist(const T& vertices, std::ostream& output) const;
    size_t apaxve() const; // 1 + max vertex no.
    size_t lowerboundv(const size_t start = 0) const;
    std::set<size_t> vset_all() const;
    std::set<size_t> vnt(const size_t type) const;
    void addV(const std::vector<TVertex>& vertices); // Overloading for single vertex is always bad, pointless or both.
    std::set<size_t> vnv(const size_t v1, const size_t v2) const; //vset newset vset v1 v2
    std::set<size_t> vnc(const std::set<size_t>& cset) const; //vset newset cset
    std::set<size_t> validated_vset(const std::set<size_t>& vset) const;

    std::vector<std::unique_ptr<TCell>> global_cells;
    void c(const size_t cellNumber, const std::vector<size_t>& vertices, const size_t type);
    void cdelAll();
    void cdel(const size_t cellNumber);
    template <typename T> void clist(const T& cells, std::ostream& output) const;
    size_t apaxce() const; // 1 + max cell no.
    size_t lowerboundc(const size_t start = 0) const;
    std::set<size_t> cset_all() const;
    std::set<size_t> cnt(const size_t type) const;
    void addC(const std::vector<TCell>& cells);
    std::set<size_t> cnc(const size_t c1, const size_t c2) const; //cset newset cset c1 c2
    std::set<size_t> cnv(const std::set<size_t>& vset) const; //cset newset vset
    std::set<size_t> validated_cset(const std::set<size_t>& cset) const;

    std::vector<std::unique_ptr<TVertexType>> global_vertexTypes;
    void vt(const size_t typeNumber);
    void vtdelAll();
    void vtdel(const size_t typeNumber);
    template <typename T> void vtlist(const T& vertexTypes, std::ostream& output) const;
    size_t apaxvt() const; // 1 + max vertex type no.
    size_t lowerboundvt(const size_t start = 0) const;
    std::set<size_t> vtset_all() const;
    bool vit(const size_t type) const; // true if type is defined for any vertex
    std::set<size_t> validated_vtset(const std::set<size_t>& vtset) const;

    std::vector<std::unique_ptr<TCellType>> global_cellTypes;
    void ct(const size_t typeNumber);
    void ctdelAll();
    void ctdel(const size_t typeNumber);
    template <typename T> void ctlist(const T& cellTypes, std::ostream& output) const;
    size_t apaxct() const; // 1 + max cell type no.
    size_t lowerboundct(const size_t start = 0) const;
    std::set<size_t> ctset_all() const;
    bool cit(const size_t type) const; // true if type is defined for any cell
    std::set<size_t> validated_ctset(const std::set<size_t>& ctset) const;

    std::vector<TSurface> vsurfaces;
    std::vector<TSurface> findSurfaces(const size_t cellNumber) const;
    void update_vsurfaces(std::unordered_set<size_t> cset);

    TModel() {}

    void delModel();
};

template <typename T> void TModel::vlist(const T& vertices, std::ostream& output) const {
#if defined (_MSC_VER)
    _set_output_format(_TWO_DIGIT_EXPONENT); // set digits after exponent to two. It is on VS, already default on gcc.
#endif
    output << std::setw(12) << "vertex no." << std::setw(12) << "type" << std::setw(18) << "x" << std::setw(18) << "y" << std::setw(18) << "z" << std::endl;
    size_t counter = 0;
    for (typename T::const_iterator i = vertices.cbegin(); i != vertices.cend(); ++i) {
        if (*i < global_vertices.size() && global_vertices[*i]) {
            output << std::setw(12) << *i << std::setw(12) << global_vertices[*i]->type << std::setw(18) << global_vertices[*i]->x
                << std::setw(18) << global_vertices[*i]->y << std::setw(18) << global_vertices[*i]->z << std::endl;
            ++counter;
        }
    }
}

template <typename T> void TModel::clist(const T& cells, std::ostream& output) const {
    output << std::setw(12) << "cell no." << std::setw(12) << "type" << std::setw(12) << "v1" << std::setw(12) << "v2" << std::setw(12) << "v3" << std::setw(12) << "v4" 
        << std::setw(12) << "v5" << std::setw(12) << "v6" << std::setw(12) << "v7" << std::setw(12) << "v8" << std::endl;
    size_t counter = 0;
    for (typename T::const_iterator i = cells.cbegin(); i != cells.cend(); ++i) {
        if (*i < global_cells.size() && global_cells[*i]) {
            output << std::setw(12) << *i << std::setw(12) << global_cells[*i]->type;
            for (std::vector<size_t>::const_iterator j = global_cells[*i]->vertices.cbegin(); j != global_cells[*i]->vertices.cend(); ++j)
                output << std::setw(12) << *j;
            output << std::endl;
            ++counter;
        }
    }
}

template <typename T> void TModel::vtlist(const T& vertexTypes, std::ostream& output) const {
    output << std::setw(12) << "v. type no." << std::setw(14) << "name" << std::endl;
    size_t counter = 0;
    for (typename T::const_iterator i = vertexTypes.cbegin(); i != vertexTypes.cend(); ++i) {
        if (*i < global_vertexTypes.size() && global_vertexTypes[*i]) {
            output << std::setw(12) << *i << std::setw(14) << global_vertexTypes[*i]->name << std::endl;
            ++counter;
        }
    }
}

template <typename T> void TModel::ctlist(const T& cellTypes, std::ostream& output) const {
    output << std::setw(12) << "c. type no." << std::setw(12) << "dimension" << std::setw(12) << "ansysType" 
        << std::setw(12) << "abaqusType" << std::setw(12) << "mType" << std::setw(12) << "realConst." 
        << std::setw(12) << "cellCsys." << std::setw(14) << "name" << std::endl;
    size_t counter = 0;
    for (typename T::const_iterator i = cellTypes.cbegin(); i != cellTypes.cend(); ++i) {
        if (*i < global_cellTypes.size() && global_cellTypes[*i]) {
            output << std::setw(12) << *i << std::setw(12) << global_cellTypes[*i]->dimension << std::setw(12) << global_cellTypes[*i]->ansysType
                << std::setw(12) << global_cellTypes[*i]->abaqusType << std::setw(12) << global_cellTypes[*i]->mType << std::setw(12) << global_cellTypes[*i]->realConstant 
                << std::setw(12) << global_cellTypes[*i]->cellCSys << std::setw(14) << global_cellTypes[*i]->name  << std::endl;
            ++counter;
        }
    }
}

struct TVector {
    double x, y, z;
};

void surfPatch(TModel& myModel, const size_t div1, const size_t div2, const size_t v1, const size_t v2, const size_t v3, const size_t v4, const size_t vtype, const size_t ctype);
std::set<size_t> squareSectionPatch(TModel& myModel, const size_t v1, const size_t v2, unsigned long div1, unsigned long div2, unsigned long div3, 
                                    TVector u11, TVector u12, TVector u21, TVector u22, const size_t type);

class TMap4 {
    double a, b, c, d;
public:
    TMap4(double x1, double x2, double x3, double x4) {
        a = x1;
        b = x2 - a;
        c = x4 - a;
        d = x3 - a - b - c;
    }
    double operator () (const double y1, const double y2) const {
        return a + b * y1 + c * y2 + d * y1 * y2;
    }
    /*x = a + b*y1 + c*y2 + d*y1 * y2
    (at y1 = y2 = 0) x1 = a
    (at y1 = 1; y2 = 0) x2 = a + b
    (at y1 = 0; y2 = 1) x4 = a + c
    (at y1 = y2 = 1) x3 = a + b + c + d
    */
};

typedef std::unordered_set<TEdge, TEdge::hasher> TCoord;

struct TCellVert {
    size_t cell;
    size_t index;
    TCellVert(size_t acell, size_t anindex): cell(acell), index(anindex) {};
};

struct TCellVerts {
    std::vector<TCellVert> items;
    bool coordinated;
    TCellVerts(): coordinated(false) {};
};

class TCoordMesh2D {
private:
    TModel& aModel;
    std::mutex coord_mutex;
    std::mutex threadsvector_mutex;
    std::list<std::thread> coord_threads;
    std::unordered_map<TEdge, TCellVerts, TEdge::hasher> Edges;
    void continueCoord(const size_t cellNumber, const size_t index, TCoord& aCoord);
    TEdge findEdge(const size_t cellNumber, const size_t index) const;
    void addtoCoord(const TEdge& anEdge, TCoord& aCoord);
    std::unordered_map<size_t, std::set<size_t>> cell_index(const TCoord& aCoord) const;
    size_t addmidV(const size_t cellNumber, const size_t index, const size_t vnumber);
    void addcenterV(const size_t cellNumber, const size_t vnumber);
    std::unordered_map<TEdge, size_t, TEdge::hasher> newmidvertices;
    std::vector<size_t> newcentervertices;
    std::vector<size_t> newcells;
    std::vector<size_t> delcells;
public:
    TCoordMesh2D(TModel& myModel): aModel(myModel) {};
    std::vector<TCoord> coords;
    void updateEdges();
    void updateCoords();
    TCoord getCoord(const TEdge& anEdge);
    std::set<size_t> csla(const TCoord &aCoord) const;
    std::string cell_str(const TCoord& aCoord) const;
    void sp2D(const size_t cellNumber, const std::vector<size_t>& indices);
    std::vector<std::vector<size_t>> divc(const TEdge& cutEdge);
};

#endif	/* MESH_H */
