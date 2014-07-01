/*
* File:   celltops.cpp
* Author: Saeid Emami
*
* Created on June 27, 2013, 1:08 AM
*/

#include "Mesh.h"
#include <exception>

class bad_cellt_alloc: public std::bad_alloc
{
    virtual const char* what() const throw()
    {
        return "Error allocating memory for cell type vector.";
    }
};

void TModel::ct(const size_t typeNumber) {
    if (global_cellTypes.size() <= typeNumber) {
        size_t maxSize = global_cellTypes.max_size();
        if (typeNumber >= maxSize) {
            bad_cellt_alloc e;
            throw e;
        }
        size_t newSize = 2 * global_cellTypes.size();
        if (global_cellTypes.size() == 0)
            newSize = 1;
        while (newSize <= typeNumber)
            newSize *= 2;
        try {
            global_cellTypes.resize(newSize <= maxSize ? newSize : maxSize);
        }
        catch (std::bad_alloc&) {
            bad_cellt_alloc e;
            throw e;
        }
    }
    if (!global_cellTypes[typeNumber])
        global_cellTypes[typeNumber] = std::unique_ptr<TCellType> (new TCellType);
}

void TModel::ctdelAll() {
    global_cellTypes.clear();
    std::vector<std::unique_ptr<TCellType>>().swap(global_cellTypes);
}

void TModel::ctdel(const size_t typeNumber) {
    global_cellTypes[typeNumber].reset();
}

size_t TModel::apaxct() const {
    size_t upperValue = global_cellTypes.size();
    while (upperValue>0 && !global_cellTypes[upperValue - 1])
        --upperValue;
    return upperValue;
}

size_t TModel::lowerboundct(const size_t start) const {
    size_t result = start;
    while (result < global_cellTypes.size() && global_cellTypes[result])
        ++result;
    return result;
}

std::set<size_t> TModel::ctset_all() const {
    std::set<size_t> result;
    for (size_t i = 0; i != global_cellTypes.size(); ++i)
        if (global_cellTypes[i])
            result.insert(i); 
    return result;
}

std::set<size_t> TModel::validated_ctset(const std::set<size_t>& ctset) const {
    std::set<size_t> result;
    for(std::set<size_t>::const_iterator i = ctset.cbegin(); i != ctset.cend(); ++i)
        if (*i <= global_cellTypes.size() && global_cellTypes[*i])
            result.insert(result.end(), *i);
    return result;
}
