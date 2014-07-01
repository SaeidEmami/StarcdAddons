/*
* File:   cellops.cpp
* Author: Saeid Emami
*
* Created on June 27, 2013, 12:57 AM
*/

#include "Mesh.h"
#include <exception>

class bad_cell_alloc: public std::bad_alloc
{
  virtual const char* what() const throw()
  {
    return "Error allocating memory for cell vector.";
  }
};

void TModel::c(const size_t cellNumber, const std::vector<size_t>& vertices, const size_t type) {
    if (global_cells.size() <= cellNumber) {
        size_t maxSize = global_cells.max_size();
        if (cellNumber >= maxSize) {
            bad_cell_alloc e;
            throw e;
        }
        size_t newSize = 2 * global_cells.size();
        if (global_cells.size() == 0)
            newSize = 1;
        while (newSize <= cellNumber)
            newSize *= 2;
        try {
            global_cells.resize(newSize <= maxSize ? newSize : maxSize);
        }
        catch (std::bad_alloc&) {
            bad_cell_alloc e;
            throw e;
        }
    }
    if (!global_cells[cellNumber])
        global_cells[cellNumber] = std::unique_ptr<TCell> (new TCell);
    global_cells[cellNumber]->vertices = vertices;
    global_cells[cellNumber]->type = type;
}

void TModel::cdelAll() {
    global_cells.clear();
    std::vector<std::unique_ptr<TCell>>().swap(global_cells);
}

void TModel::cdel(const size_t cellNumber) {
    global_cells[cellNumber].reset();
}

size_t TModel::apaxce() const {
    size_t upperValue = global_cells.size();
    while (upperValue > 0 && !global_cells[upperValue - 1])
        --upperValue;
    return upperValue;
}

size_t TModel::lowerboundc(const size_t start) const {
    size_t result = start;
    while (result < global_cells.size() && global_cells[result])
        ++result;
    return result;
}

std::set<size_t> TModel::cset_all() const {
    std::set<size_t> result;
    for (size_t i = 0; i != global_cells.size(); ++i)
        if (global_cells[i])
            result.insert(i); 
    return result;
}

std::set<size_t> TModel::cnt(const size_t type) const {
    std::set<size_t> result;
    for (size_t i = 0; i < global_cells.size(); ++i)
        if (global_cells[i] && global_cells[i]->type == type)
            result.insert(result.end(), i);
    return result;
}

bool TModel::cit(const size_t type) const { // true if type is defined for any cell
    for (size_t i = 0; i < global_cells.size(); ++i)
        if (global_cells[i] && global_cells[i]->type == type)
            return true;
    return false;
}

void TModel::addC(const std::vector<TCell>& cells) {
    size_t number = 0;
    for (size_t i = 0; i < cells.size(); ++i) {
        number = lowerboundc(number);
        c(number, cells[i].vertices, cells[i].type);
    }
}

std::set<size_t> TModel::cnc(const size_t c1, const size_t c2) const {
    std::set<size_t> result;
    size_t lastc = c2 <= (global_cells.size() - 1)? c2 : (global_cells.size() - 1);
    for (size_t i = c1; i <= lastc; ++i)
        if (global_cells[i])
            result.insert(result.end(), i);
    return result;
}

std::set<size_t> TModel::cnv(const std::set<size_t>& vset) const {
    std::set<size_t> result;
    for (size_t i = 0; i < global_cells.size(); ++i)
        if (global_cells[i])
            for (std::vector<size_t>::const_iterator v = global_cells[i]->vertices.cbegin(); v != global_cells[i]->vertices.cend(); ++v)
                if (vset.count(*v))
                    result.insert(result.end(), i);
    return result;
}

std::set<size_t> TModel::validated_cset(const std::set<size_t>& cset) const {
    std::set<size_t> result;
    for(std::set<size_t>::const_iterator i = cset.cbegin(); i != cset.cend(); ++i)
        if (*i <= global_cells.size() && global_cells[*i])
            result.insert(result.end(), *i);
    return result;
}

