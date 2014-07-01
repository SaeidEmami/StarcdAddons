/*
* File:   vertexops.cpp
* Author: Saeid Emami
*
* Created on June 27, 2013, 12:13 AM
*/

#include "Mesh.h"
#include <exception>

class bad_vertex_alloc: public std::bad_alloc
{
    virtual const char* what() const throw()
    {
        return "Error allocating memory for vertex vector.";
    }
};

void TModel::v(const size_t vertexNumber, const double x, const double y, const double z, const size_t type) {
    if (global_vertices.size() <= vertexNumber) {
        size_t maxSize = global_vertices.max_size();
        if (vertexNumber >= maxSize) {
            bad_vertex_alloc e;
            throw e;
        }
        size_t newSize = 2 * global_vertices.size();
        if (global_vertices.size() == 0)
            newSize = 1;
        while (newSize <= vertexNumber)
            newSize *= 2;
        try {
            global_vertices.resize(newSize <= maxSize ? newSize : maxSize);
        }
        catch (std::bad_alloc&) {
            bad_vertex_alloc e;
            throw e;
        }
    }
    if (!global_vertices[vertexNumber])
        global_vertices[vertexNumber] = std::unique_ptr<TVertex> (new TVertex);
    global_vertices[vertexNumber]->x = x;
    global_vertices[vertexNumber]->y = y;
    global_vertices[vertexNumber]->z = z;
    global_vertices[vertexNumber]->type = type;
}

void TModel::vdelAll() {
    global_vertices.clear();
    std::vector<std::unique_ptr<TVertex>>().swap(global_vertices);
}

void TModel::vdel(const size_t vertexNumber) {
    global_vertices[vertexNumber].reset();
}

size_t TModel::apaxve() const {
    size_t upperValue = global_vertices.size();
    while (upperValue > 0 && !global_vertices[upperValue - 1])
        --upperValue;
    return upperValue;
}

size_t TModel::lowerboundv(const size_t start) const {
    size_t result = start;
    while (result < global_vertices.size() && global_vertices[result])
        ++result;
    return result;
}

std::set<size_t> TModel::vset_all() const {
    std::set<size_t> result;
    for (size_t i = 0; i != global_vertices.size(); ++i)
        if (global_vertices[i])
            result.insert(i); 
    return result;
}

std::set<size_t> TModel::vnt(const size_t type) const {
    std::set<size_t> result;
    for (size_t i = 0; i < global_vertices.size(); ++i)
        if (global_vertices[i] && global_vertices[i]->type == type)
            result.insert(result.end(), i);
    return result;
}

bool TModel::vit(const size_t type) const { // true if type is defined for any vertex
    for (size_t i = 0; i < global_vertices.size(); ++i)
        if (global_vertices[i] && global_vertices[i]->type == type)
            return true;
    return false;
}

void TModel::addV(const std::vector<TVertex>& vertices) {
    size_t number = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        number = lowerboundv(number);
        v(number, vertices[i].x, vertices[i].y, vertices[i].z, vertices[i].type);
    }
}

std::set<size_t> TModel::vnv(const size_t v1, const size_t v2) const {
    std::set<size_t> result;
    size_t lastv = v2 <= (global_vertices.size() - 1)? v2 : (global_vertices.size() - 1);
    for (size_t i = v1; i <= lastv; ++i)
        if (global_vertices[i])
            result.insert(result.end(), i); 
    return result;
}

std::set<size_t> TModel::vnc(const std::set<size_t>& cset) const {
    std::set<size_t> result;
    for (std::set<size_t>::const_iterator i = cset.cbegin(); i != cset.cend(); ++i)
        if (global_cells[*i])
            for (std::vector<size_t>::const_iterator v = global_cells[*i]->vertices.cbegin(); v != global_cells[*i]->vertices.cend(); ++v)
                result.insert(*v);
    return result;
}

std::set<size_t> TModel::validated_vset(const std::set<size_t>& vset) const {
    std::set<size_t> result;
    for(std::set<size_t>::const_iterator i = vset.cbegin(); i != vset.cend(); ++i)
        if (*i <= global_vertices.size() && global_vertices[*i])
            result.insert(result.end(), *i);
    return result;
}



