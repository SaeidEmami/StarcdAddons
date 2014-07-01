/*
* File:   vertexops.cpp
* Author: Saeid Emami
*
* Created on June 27, 2013, 1:05 AM
*/

#include "Mesh.h"
#include <exception>

class bad_vertext_alloc: public std::bad_alloc
{
  virtual const char* what() const throw()
  {
    return "Error allocating memory for vertex type vector.";
  }
};

void TModel::vt(const size_t typeNumber) {
    if (global_vertexTypes.size() <= typeNumber) {
        size_t maxSize = global_vertexTypes.max_size();
        if (typeNumber >= maxSize) {
            bad_vertext_alloc e;
            throw e;
        }
        size_t newSize = 2 * global_vertexTypes.size();
        if (global_vertexTypes.size() == 0)
            newSize = 1;
        while (newSize <= typeNumber)
            newSize *= 2;
        try {
            global_vertexTypes.resize(newSize <= maxSize ? newSize : maxSize);
        }
        catch (std::bad_alloc&) {
            bad_vertext_alloc e;
            throw e;
        }
    }
    if (!global_vertexTypes[typeNumber])
        global_vertexTypes[typeNumber] = std::unique_ptr<TVertexType> (new TVertexType);
}

void TModel::vtdelAll() {
    global_vertexTypes.clear();
    std::vector<std::unique_ptr<TVertexType>>().swap(global_vertexTypes);
}

void TModel::vtdel(const size_t typeNumber) {
    global_vertexTypes[typeNumber].reset();
}

size_t TModel::apaxvt() const {
    size_t upperValue = global_vertexTypes.size();
    while (upperValue > 0 && !global_vertexTypes[upperValue - 1])
        --upperValue;
    return upperValue;
}

size_t TModel::lowerboundvt( const size_t start) const {
    size_t result = start;
    while (result < global_vertexTypes.size() && global_vertexTypes[result])
        ++result;
    return result;
}

std::set<size_t> TModel::vtset_all() const {
    std::set<size_t> result;
    for (size_t i = 0; i != global_vertexTypes.size(); ++i)
        if (global_vertexTypes[i])
            result.insert(i); 
    return result;
}

std::set<size_t> TModel::validated_vtset(const std::set<size_t>& vtset) const {
    std::set<size_t> result;
    for(std::set<size_t>::const_iterator i = vtset.cbegin(); i != vtset.cend(); ++i)
        if (*i <= global_vertexTypes.size() && global_vertexTypes[*i])
            result.insert(result.end(), *i);
    return result;
}
