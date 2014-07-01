/*
* File:   Mesh.cpp
* Author: Saeid Emami
*
* Created on June 6, 2013, 7:21 PM
*/

#include "Mesh.h"
#include <sstream>
#include <thread>
#include <cassert>

void TModel::delModel() {
    vdelAll();
    cdelAll();
    vtdelAll();
    ctdelAll();
}

std::vector<TSurface> TModel::findSurfaces(const size_t cellNumber) const {
    std::vector<TSurface> result;
    if (cellNumber >= global_cells.size() || !global_cells[cellNumber]) 
        return result;
    size_t cell_t = global_cells[cellNumber]->type;
    if (global_cellTypes[cell_t]) {
        if (global_cellTypes[cell_t]->dimension == 2) { // 2D cells
            result.push_back(TSurface(global_cells[cellNumber]->vertices));
        }
        else if (global_cellTypes[cell_t]->dimension == 3) { // 3D cells
            size_t vsize = global_cells[cellNumber]->vertices.size();
            if (vsize == 4) {
                std::vector<size_t> V0(3), V1(3), V2(3), V3(3);
                for (size_t i = 0; i < 3; ++i) {
                    V0[i] = global_cells[cellNumber]->vertices[i];
                    V1[i] = global_cells[cellNumber]->vertices[(i + 1) % 4];
                    V2[i] = global_cells[cellNumber]->vertices[(i + 2) % 4];
                    V3[i] = global_cells[cellNumber]->vertices[(i + 3) % 4];
                }
                result.push_back(TSurface(V0));
                result.push_back(TSurface(V1));
                result.push_back(TSurface(V2));
                result.push_back(TSurface(V3));
            }
            else if (vsize == 5) { // pyramid
                size_t v0 = global_cells[cellNumber]->vertices[0];
                size_t v1 = global_cells[cellNumber]->vertices[1];
                size_t v2 = global_cells[cellNumber]->vertices[2];
                size_t v3 = global_cells[cellNumber]->vertices[3];
                size_t v4 = global_cells[cellNumber]->vertices[4];
                std::vector<size_t> V0(4), V1(3), V2(3), V3(3), V4(3);;
                V0[0] = v0; V0[1] = v1; V0[2] = v2; V0[3] = v3;
                V1[0] = v0; V1[1] = v1; V1[2] = v4;
                V2[0] = v1; V2[1] = v2; V2[2] = v4;
                V3[0] = v2; V3[1] = v3; V3[2] = v4;
                V4[0] = v3; V4[1] = v0; V4[2] = v4;
                result.push_back(TSurface(V0));
                result.push_back(TSurface(V1));
                result.push_back(TSurface(V2));
                result.push_back(TSurface(V3));
                result.push_back(TSurface(V4));
            }
            else if (vsize == 6) { // prism
                size_t v0 = global_cells[cellNumber]->vertices[0];
                size_t v1 = global_cells[cellNumber]->vertices[1];
                size_t v2 = global_cells[cellNumber]->vertices[2];
                size_t v3 = global_cells[cellNumber]->vertices[3];
                size_t v4 = global_cells[cellNumber]->vertices[4];
                size_t v5 = global_cells[cellNumber]->vertices[5];
                std::vector<size_t> V0(3), V1(4), V2(4), V3(4), V4(3);
                V0[0] = v0; V0[1] = v1; V0[2] = v2;
                V1[0] = v0; V1[1] = v1; V1[2] = v4; V1[3] = v3; 
                V2[0] = v1; V2[1] = v2; V2[2] = v5; V2[3] = v4;
                V3[0] = v2; V3[1] = v0; V3[2] = v3; V3[3] = v5;
                V4[0] = v3; V4[1] = v4; V4[2] = v5;
                result.push_back(TSurface(V0));
                result.push_back(TSurface(V1));
                result.push_back(TSurface(V2));
                result.push_back(TSurface(V3));
                result.push_back(TSurface(V4));
            }
            else if (vsize == 8) { // common block
                size_t v0 = global_cells[cellNumber]->vertices[0];
                size_t v1 = global_cells[cellNumber]->vertices[1];
                size_t v2 = global_cells[cellNumber]->vertices[2];
                size_t v3 = global_cells[cellNumber]->vertices[3];
                size_t v4 = global_cells[cellNumber]->vertices[4];
                size_t v5 = global_cells[cellNumber]->vertices[5];
                size_t v6 = global_cells[cellNumber]->vertices[5];
                size_t v7 = global_cells[cellNumber]->vertices[5];
                std::vector<size_t> V0(4), V1(4), V2(4), V3(4), V4(4), V5(4);
                V0[0] = v0; V0[1] = v1; V0[2] = v2; V0[3] = v3;
                V1[0] = v4; V1[1] = v5; V1[2] = v6; V1[3] = v7; 
                V2[0] = v0; V2[1] = v1; V2[2] = v5; V2[3] = v4;
                V3[0] = v1; V3[1] = v2; V3[2] = v6; V3[3] = v5;
                V4[0] = v2; V4[1] = v3; V4[2] = v7; V4[3] = v6;
                V5[0] = v3; V5[1] = v0; V5[2] = v4; V5[3] = v7;
                result.push_back(TSurface(V0));
                result.push_back(TSurface(V1));
                result.push_back(TSurface(V2));
                result.push_back(TSurface(V3));
                result.push_back(TSurface(V4));
                result.push_back(TSurface(V5));
            }
        }
    }
    return result;
}

void TModel::update_vsurfaces(std::unordered_set<size_t> cset) {
    for (std::unordered_set<size_t>::const_iterator i = cset.cbegin(); i != cset.cend(); ++i) {
        std::vector<TSurface> result = findSurfaces(*i);
        vsurfaces.insert(vsurfaces.end(), result.cbegin(), result.cend());
    }
}

void surfPatch(TModel& myModel, const size_t divi, const size_t divj, const size_t v1, const size_t v2, const size_t v3, const size_t v4, const size_t vtype, const size_t ctype) {
    if (v1 >= myModel.global_vertices.size() || v2 >= myModel.global_vertices.size() || v3 >= myModel.global_vertices.size() || v4 >= myModel.global_vertices.size() || 
        !myModel.global_vertices[v1] || !myModel.global_vertices[v2] || !myModel.global_vertices[v3] || !myModel.global_vertices[v4]) {
            std::logic_error e("Error: Undefined vertex.");
            throw e;
    }
    if (divi <= 0 || divj <= 0) {
        std::logic_error e("Error: Invalid division count.");
        throw e;
    }
    TVector aVertex;
    TCell aCell;
    size_t vnumber = myModel.apaxve(), cnumber = myModel.apaxce();
    std::vector<std::vector <size_t>> vertexNumbers(divi + 1, std::vector<size_t>(divj + 1, 0));
    std::vector<size_t> cellVertices(4);
    double etha = 0, epsilon = 0;
    TMap4 mapx(myModel.global_vertices[v1]->x, myModel.global_vertices[v2]->x, myModel.global_vertices[v3]->x, myModel.global_vertices[v4]->x);
    TMap4 mapy(myModel.global_vertices[v1]->y, myModel.global_vertices[v2]->y, myModel.global_vertices[v3]->y, myModel.global_vertices[v4]->y);
    TMap4 mapz(myModel.global_vertices[v1]->z, myModel.global_vertices[v2]->z, myModel.global_vertices[v3]->z, myModel.global_vertices[v4]->z);
    for (int j = 0; j <= divj; ++j) {
        etha = (double) j / divj;
        for (int i = 0; i <= divi; ++i) {
            epsilon = (double) i / divi;
            etha = (double) j / divj;
            vertexNumbers[i][j] = vnumber;
            myModel.v(vnumber, mapx(etha, epsilon), mapy(etha, epsilon), mapz(etha, epsilon), vtype);
            ++vnumber;
            if (i > 0 && j > 0) {
                cellVertices[0] = vertexNumbers[i - 1][j - 1];
                cellVertices[1] = vertexNumbers[i][j - 1];
                cellVertices[2] = vertexNumbers[i][j];
                cellVertices[3] = vertexNumbers[i - 1][j];
                myModel.c(cnumber++, cellVertices, ctype);
            }
        }
    }
}

TEdge::TEdge(const size_t v1, const size_t v2){
    assignVertices(v1, v2);
}

TEdge::TEdge(const TEdge& t) {
    V1 = t.V1;
    V2 = t.V2;
}

TEdge& TEdge::operator=(const TEdge& t) {
    V1 = t.V1;
    V2 = t.V2;
    return *this;
}

void TEdge::assignVertices(const size_t v1, const size_t v2) {
    if (v1 <= v2) {
        V1 = v1;
        V2 = v2;
    }
    else {
        V1 = v2;
        V2 = v1;
    }
}

bool TEdge::operator== (const TEdge& t) const {
    return (V1 == t.V1 && V2 == t.V2);
}

bool TEdge::operator!= (const TEdge& t) const {
    return (V1 != t.V1 || V2 != t.V2);
}

bool TEdge::operator< (const TEdge& t) const {
    if (V1 < t.V1)
        return true;
    else if (V1 == t.V1)
        return (V2 < t.V2);
    else
        return false;
}

std::string TEdge::str() const {
    std::stringstream astream;
    astream << "(" << V1 << ", " << V2 << ")" << std::flush;
    return astream.str();  
}

TSurface::TSurface(const std::vector<size_t>& v) {
    assignVertices(v);
}

TSurface::TSurface(const TSurface& s) {
    V = s.V;
}

TSurface& TSurface::operator=(const TSurface& s) {
    V = s.V;
    return *this;
}

void TSurface::assignVertices(const std::vector<size_t>& v) {
    if (v.size() == 0) return;
    size_t minindex = 0, minv = v[0];
    for (size_t i = 1; i < v.size(); ++i) {
        if (v[i] <= minv) {
            minindex = i;
            minv = v[i];
        }
    }
    V.resize(v.size());
    for (size_t i = 0; i < v.size(); ++i)
        V[i] = v[(i + minv) % v.size()];
}

bool TSurface::operator== (const TSurface& s) const {
    if (s.V.size() == V.size()) {
        for (size_t i = 0; i < V.size(); ++i)
            if (s.V[i] != V[i]) return false;
        return true;
    }
    return false;
}

bool TSurface::operator!= (const TSurface& s) const {
    if (s.V.size() == V.size()) {
        for (size_t i = 0; i < V.size(); ++i)
            if (s.V[i] != V[i]) return true;
        return false;
    }
    return true;
}

bool TSurface::operator< (const TSurface& s) const {
    if (V.size() < s.V.size()) return true;
    else if (V.size() > s.V.size()) return false;
    size_t i = 0;
    while (i < V.size()) {
        if (V[i] < s.V[i])
            return true;
        else if (V[i] > s.V[i])
            return false;
        ++i;
    }
    return false;
}

void TSurface::flip() {
    if (V.size() <= 2) return;
    std::vector<size_t> newV(V.size());
    for (size_t i = 1; i < V.size(); ++i)
        newV[i] = V[V.size() - i];
    V = std::move(newV);
}

#include <iostream>

void TCoordMesh2D::updateEdges() {
    std::unordered_map<TEdge, TCellVerts, TEdge::hasher> local_Edges(aModel.global_cells.size());
    for (size_t i = 0; i < aModel.global_cells.size(); ++i) {
        if (aModel.global_cells[i]) {
            size_t cell_t = aModel.global_cells[i]->type;
            if (aModel.global_cellTypes[cell_t] && aModel.global_cellTypes[cell_t]->dimension == 2) { // 2D cell
                size_t vsize = aModel.global_cells[i]->vertices.size();
                for (size_t j = 0; j < vsize; ++j) {
                    size_t vnumber = aModel.global_cells[i]->vertices[j];
                    local_Edges[findEdge(i, j)].items.push_back(TCellVert(i, j));
                }
            }
        }
    }
    Edges = std::move(local_Edges);
}

TEdge TCoordMesh2D::findEdge(const size_t cellNumber, const size_t index) const {
    TEdge result;
    size_t vsize = aModel.global_cells[cellNumber]->vertices.size();
    if (index > 0)
        result.assignVertices(aModel.global_cells[cellNumber]->vertices[index - 1], aModel.global_cells[cellNumber]->vertices[index]);
    else
        result.assignVertices(aModel.global_cells[cellNumber]->vertices[0], aModel.global_cells[cellNumber]->vertices.back());
    return result;
}

void TCoordMesh2D::addtoCoord(const TEdge& anEdge, TCoord& aCoord) {
    coord_mutex.lock();
    aCoord.insert(anEdge);
    Edges[anEdge].coordinated = true;
    coord_mutex.unlock();
}

void TCoordMesh2D::continueCoord(const size_t cellNumber, const size_t index, TCoord& aCoord) {
    bool isCoordStarted = false;
    size_t newCell = cellNumber, newIndex = index;
    size_t newCell_thread, newIndex_thread;
    TEdge firstEdge;
    TCellVerts edgeData;
    while (true) {
        firstEdge = findEdge(newCell, newIndex);
        edgeData = Edges[firstEdge];
        if (edgeData.coordinated)
            return;
        addtoCoord(firstEdge, aCoord);
        std::vector<TCellVert> valid_cellV;
        for (std::vector<TCellVert>::iterator i = edgeData.items.begin(); i != edgeData.items.end(); ++i) {
            if ((i->cell != newCell) && (aModel.global_cells[i->cell]->vertices.size() % 2 == 0 ))
                valid_cellV.push_back(*i);
        }
        if (!isCoordStarted) {
            if (aModel.global_cells[newCell]->vertices.size() % 2 == 0 ) {
                newIndex = (newIndex + aModel.global_cells[newCell]->vertices.size() /2) % aModel.global_cells[newCell]->vertices.size();
                isCoordStarted = true;
            }
            else
                return;
        }
        else {
            if (valid_cellV.size() == 0)
                return;
            std::vector<TCellVert>::iterator lastVData = valid_cellV.end() - 1;
            newCell = lastVData->cell;
            newIndex = (lastVData->index + aModel.global_cells[newCell]->vertices.size() /2) % aModel.global_cells[newCell]->vertices.size();
            valid_cellV.erase(lastVData);
        }
        for (std::vector<TCellVert>::iterator i = valid_cellV.begin(); i != valid_cellV.end(); ++i) {
            newCell_thread = i->cell;
            newIndex_thread = (i->index + aModel.global_cells[i->cell]->vertices.size() /2) % aModel.global_cells[i->cell]->vertices.size();
            std::thread t(&TCoordMesh2D::continueCoord, this, newCell_thread, newIndex_thread, std::ref(aCoord));
            threadsvector_mutex.lock();
            coord_threads.push_back(std::move(t));
            threadsvector_mutex.unlock();
        }
    }
}

void TCoordMesh2D::updateCoords() {
    for(std::unordered_map<TEdge, TCellVerts, TEdge::hasher>::const_iterator i = Edges.cbegin(); i != Edges.cend(); ++i) {
        if (!i->second.coordinated) {
            TCoord aCoord;
            coord_threads.clear();
            continueCoord(i->second.items.back().cell, i->second.items.back().index, aCoord);
            for (std::list<std::thread>::iterator t = coord_threads.begin(); t != coord_threads.end(); ++t)
                t->join();
            coords.push_back(aCoord);
        }
    }
}

TCoord TCoordMesh2D::getCoord(const TEdge& anEdge) {
    TCoord result;
    std::unordered_map<TEdge, TCellVerts, TEdge::hasher>::iterator i = Edges.find(anEdge);
    if (i != Edges.cend() && i->second.items.size() > 0 ) {
        coord_threads.clear();
        continueCoord(i->second.items.back().cell, i->second.items.back().index, result);
        for (std::list<std::thread>::iterator t = coord_threads.begin(); t != coord_threads.end(); ++t)
            t->join();
    }
    return result;
}

std::set<size_t> TCoordMesh2D::csla(const TCoord& aCoord) const {
    std::set<size_t> result;
    for (TCoord::const_iterator i = aCoord.cbegin(); i != aCoord.cend(); ++i) {
        for (std::vector<TCellVert>::const_iterator j = Edges.at(*i).items.cbegin(); j != Edges.at(*i).items.cend(); ++j)
            result.insert(j->cell);
    }
    return result;
}

std::string TCoordMesh2D::cell_str(const TCoord& aCoord) const {
    std::stringstream result;
    for (TCoord::const_iterator i = aCoord.cbegin(); i != aCoord.cend(); ++i) {
        result << "[";
        for (std::vector<TCellVert>::const_iterator j = Edges.at(*i).items.cbegin(); j != Edges.at(*i).items.cend(); ++j)
            result << j->cell << " ";
        result << "] "; 
    }
    return result.str();
}

std::unordered_map<size_t, std::set<size_t>> TCoordMesh2D::cell_index(const TCoord& aCoord) const {
    std::unordered_map<size_t, std::set<size_t>> result;
    for (TCoord::const_iterator i = aCoord.cbegin(); i != aCoord.cend(); ++i)
        for (std::vector<TCellVert>::const_iterator j = Edges.at(*i).items.cbegin(); j != Edges.at(*i).items.cend(); ++j)
            result[j->cell].insert(j->index);
    return std::move(result);
}

size_t TCoordMesh2D::addmidV(const size_t cellNumber, const size_t index, const size_t vnumber) {
    if (!aModel.global_cells[cellNumber] || aModel.global_cells[cellNumber]->vertices.size() <= index)
        return 0;
    size_t v1, v2;
    if (index >= 1) {
        v1 = aModel.global_cells[cellNumber]->vertices[index - 1];
        v2 = aModel.global_cells[cellNumber]->vertices[index];
    } else {
        v1 = aModel.global_cells[cellNumber]->vertices.back();
        v2 = aModel.global_cells[cellNumber]->vertices[0];
    }
    if (newmidvertices.find(TEdge(v1, v2)) != newmidvertices.end())
        return newmidvertices[TEdge(v1, v2)];
    if (!aModel.global_vertices[v1] || !aModel.global_vertices[v2])
        return 0;
    size_t newvtype = aModel.global_vertices[v1]->type;
    double newx = (aModel.global_vertices[v1]->x + aModel.global_vertices[v2]->x) / 2;
    double newy = (aModel.global_vertices[v1]->y + aModel.global_vertices[v2]->y) / 2;
    double newz = (aModel.global_vertices[v1]->z + aModel.global_vertices[v2]->z) / 2;
    aModel.v(vnumber, newx, newy, newz, newvtype);
    newmidvertices[TEdge(v1, v2)] = vnumber;
    return vnumber;
}

void TCoordMesh2D::addcenterV(const size_t cellNumber, const size_t vnumber) {
    size_t vsize = aModel.global_cells[cellNumber]->vertices.size();
    for (std::vector<size_t>::const_iterator v = aModel.global_cells[cellNumber]->vertices.cbegin(); v != aModel.global_cells[cellNumber]->vertices.cend(); ++v)
        if (!aModel.global_vertices[*v])
            return;
    size_t newvtype = aModel.global_vertices[aModel.global_cells[cellNumber]->vertices[0]]->type;
    double sumx = 0, sumy = 0, sumz = 0;
    for (std::vector<size_t>::const_iterator v = aModel.global_cells[cellNumber]->vertices.cbegin(); v != aModel.global_cells[cellNumber]->vertices.cend(); ++v) {
        sumx += aModel.global_vertices[*v]->x;
        sumy += aModel.global_vertices[*v]->y;
        sumz += aModel.global_vertices[*v]->z;
    }
    aModel.v(vnumber, sumx / vsize, sumy / vsize, sumz / vsize, newvtype);
    newcentervertices.push_back(vnumber);
}

void TCoordMesh2D::sp2D(const size_t cellNumber, const std::vector<size_t>& indices) {
    if (indices.size() == 0 || !aModel.global_cells[cellNumber])
        return;
    for (std::vector<size_t>::const_iterator index = indices.cbegin(); index != indices.cend(); ++index)
        if (aModel.global_cells[cellNumber]->vertices.size() <= *index)
            return;
    for (std::vector<size_t>::const_iterator v = aModel.global_cells[cellNumber]->vertices.cbegin(); v != aModel.global_cells[cellNumber]->vertices.cend(); ++v)
        if (!aModel.global_vertices[*v])
            return;
    size_t vsize = aModel.global_cells[cellNumber]->vertices.size();
    std::vector<std::vector<size_t>> cellsvertices;
    size_t newv = aModel.apaxve(), cell_count;
    std::set<size_t> midindices;
    if (vsize % 2 != 0) {
        std::set<size_t> frontindices;
        for (std::vector<size_t>::const_iterator indexp = indices.cbegin(); indexp != indices.cend(); ++indexp) {
            midindices.insert(*indexp);
            frontindices.insert((*indexp + (vsize + 1) / 2) % vsize);
        }
        cell_count = 2 * midindices.size();
        cellsvertices.resize(cell_count);
        for (size_t index = 0, current_cell = 0; index < vsize; ++index) {
            cellsvertices[current_cell].push_back(aModel.global_cells[cellNumber]->vertices[index]);
            if (midindices.find((index + 1) % vsize) != midindices.end()) {
                size_t vnumber = addmidV(cellNumber, (index + 1) % vsize, newv++);
                cellsvertices[current_cell].push_back(vnumber);
                current_cell = (current_cell + 1) % cell_count;
                cellsvertices[current_cell].push_back(vnumber);
            }
            if (frontindices.find((index + 1) % vsize) != frontindices.end()) {
                current_cell = (current_cell + 1) % cell_count;
                cellsvertices[current_cell].push_back(aModel.global_cells[cellNumber]->vertices[index]);
            }
        }
    }
    else {
        for (std::vector<size_t>::const_iterator indexp = indices.cbegin(); indexp != indices.cend(); ++indexp) {
            midindices.insert(*indexp);
            midindices.insert((*indexp + vsize / 2) % vsize);
        }
        cell_count = midindices.size();
        cellsvertices.resize(cell_count);
        for (size_t index = 0, current_cell = 0; index < vsize; ++index) {
            cellsvertices[current_cell].push_back(aModel.global_cells[cellNumber]->vertices[index]);
            if (midindices.find((index + 1) % vsize) != midindices.end()) {
                size_t vnumber = addmidV(cellNumber, (index + 1) % vsize, newv++);
                cellsvertices[current_cell].push_back(vnumber);
                current_cell = (current_cell + 1) % cell_count;
                cellsvertices[current_cell].push_back(vnumber);
            }
        }
    }
    if (midindices.size() > 2 || (vsize % 2 != 0 && midindices.size() == 2)){
        size_t centerv = newv++;
        addcenterV(cellNumber, centerv);
        for (size_t cellindex = 0; cellindex != cell_count; ++cellindex)
            cellsvertices[cellindex].push_back(centerv);
    }
    size_t newctype = aModel.global_cells[cellNumber]->type;
    size_t newc = aModel.apaxce();
    aModel.cdel(cellNumber);
    delcells.push_back(cellNumber);
    for (size_t cellindex = 0; cellindex != cell_count; ++cellindex) {
        newcells.push_back(newc);
        aModel.c(newc++, cellsvertices[cellindex], newctype);
    }  
}

std::vector<std::vector<size_t>> TCoordMesh2D::divc(const TEdge& cutEdge) {
    updateEdges();
    TCoord aCoord = getCoord(cutEdge);
    std::unordered_map<size_t, std::set<size_t>> indexmap = cell_index(aCoord);
    newmidvertices.clear();
    newcentervertices.clear();
    newcells.clear();
    for (std::unordered_map<size_t, std::set<size_t>>::const_iterator i = indexmap.cbegin(); i != indexmap.cend(); ++i) {
        std::vector<size_t> cutindices(i->second.cbegin(), i->second.cend());
        sp2D(i->first, cutindices);
    }
    std::vector<size_t> newvertices = std::move(newcentervertices);
    for (std::unordered_map<TEdge, size_t, TEdge::hasher>::const_iterator i = newmidvertices.cbegin(); i != newmidvertices.cend(); ++i)
        newvertices.push_back(i->second);
    std::vector<std::vector<size_t>> result;
    result.push_back(std::move(newvertices));
    result.push_back(std::move(newcells));
    result.push_back(std::move(delcells));
    return result;
}
