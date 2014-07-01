/*
* File:   IO.cpp
* Author: Saeid Emami
*
* Created on June 11, 2013, 8:16 PM
*/

#include "IO.h"
#include <fstream>
#include <cstdlib>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <algorithm>
#include <sstream>
#include <utility>
#include <thread>
#include <iostream>
#include <string>

/*std::string capitalize(const std::string& input) {
std::string result(input);
std::transform(input.begin(), input.end(), result.begin(), toupper);
return result;
}*/

std::string tokenizer::_str() {
    std::string result;
    size_t start, found;
    start = tokenline.find_first_not_of(" ,\t");

    if (start != std::string::npos) {
        found = tokenline.find_first_of(" ,\t", start);
        result = tokenline.substr(start, found - start);
        flag = true;
        if (found != std::string::npos)
            tokenline = tokenline.substr(found, std::string::npos);
        else
            tokenline = "";
    }
    else {
        result = "";
        flag = false;
    }
    return result;
}

std::string tokenizer::str() {
    ++tokencount;
    std::string result = _str();
    return result;
}

size_t tokenizer::UL() {
    ++tokencount;
    return strtoul(_str().c_str(), NULL, 10);
}

double tokenizer::D() {
    ++tokencount;
    return strtod(_str().c_str(), NULL);
}

IO_t tokenizer::IOt() {
    ++tokencount;
    std::string token(_str());
    IO_t result = UNDEFINED;
    if (token == "ANSYS")
        result = ANSYS;
    else if (token == "ABAQUS")
        result = ABAQUS;
    else if (token == "OBJECT")
        result = OBJECT;
    else if (token == "STAR")
        result = STAR;
    else
        flag = false;
    return result;
}

short starIO::dimensiontostarkey(const short dimension) const {
    switch (dimension) {
    case 0:
        return 6;  // point
    case 1:
        return 5;  // line
    case 2: 
        return 4; // shell
    case 3: 
        return 1; // solid
    default:
        return 0; // undefined
    }
}

short starIO::starkeytodimension(const short starkey) const {
    switch (starkey) {
    case 1: // solid
    case 2: // fluid
        return 3;
    case 3: // baffle
    case 4: // shell
        return 2;
    case 5: // line
        return 1;
    case 6: // point
        return 0;
    default:
        return -1; // undefined
    }
}

size_t starIO::vread(const std::string& filename) {
    std::ifstream vFile(filename); // format for .vrt files:  I9, 6X, 3G16.9; Format is strict annd should  be parsed strict
    size_t counter = 0;
    if (vFile.is_open()) {
        std::string token1(100, 0), token2(100, 0);
        size_t number = 0;
        double x = 0, y = 0, z = 0;
        vFile.read(&token1[0], 9); 
        while (vFile.good()) {
            number = strtoul(&token1[0], NULL, 10) + offset_v;
            vFile.ignore(6);
            vFile.read(&token2[0], 16);
            x = strtod(&token2[0], NULL);
            vFile.read(&token2[0], 16);
            y = strtod(&token2[0], NULL);
            vFile.read(&token2[0], 16);
            z = strtod(&token2[0], NULL);
            aModel.v(number, x, y, z, 0);
            ++counter;
            vFile.ignore(1);
            vFile.read(&token1[0], 9);
        }
    }
    return counter;
}

size_t starIO::cread(const std::string& filename) {
    std::ifstream cFile(filename); // format for .cel files: I9, 6X, 9I9, 1X, I4
    // for cells with more than 8 vertices:  (I9, 6X, 9I9, 1X, I4)*3.
    size_t counter = 0;
    if (cFile.is_open()) {
        std::string token1(100, 0), token2(100, 0), token3(100, 0);
        size_t number = 0, type = 0, vNumber = 0;
        std::vector<size_t> vertices;
        short starkey = 0, dimension = -1;        
        cFile.read(&token1[0], 15);
        while (cFile.good()) {
            number = strtoul(&token1[0], NULL, 10) + offset_c;
            vertices.clear();
            for (int i = 0; i < 8; ++i) {
                cFile.read(&token2[0], 9);
                vNumber = strtoul(&token2[0], NULL, 10);
                if (vNumber)
                    vertices.push_back(vNumber + offset_v);
            }
            cFile.read(&token2[0], 9);
            type = strtoul(&token2[0], NULL, 10);
            cFile.ignore(1);
            cFile.read(&token3[0], 4);
            starkey = strtol(&token3[0], NULL, 10);
            if (starkey >= 0)
                cFile.ignore(1);
            else { // three-line cell definiton
                cFile.ignore(16);
                for (int i = 0; i < 8 && cFile.good(); ++i) {
                    cFile.read(&token2[0], 9);
                    vNumber = strtoul(&token2[0], NULL, 10);
                    if (vNumber)
                        vertices.push_back(vNumber + offset_v);
                }
                cFile.ignore(30);
                for (int i = 0; i < 4 && cFile.good(); ++i) {
                    cFile.read(&token2[0], 9);
                    vNumber = strtoul(&token2[0], NULL, 10);
                    if (vNumber)
                        vertices.push_back(vNumber + offset_v);
                }
                cFile.ignore(27);
                token2.assign(100, 0);
                cFile.read(&token2[0], 9);
                starkey = strtol(&token2[0], NULL, 10);
                cFile.ignore(15);
            }
            aModel.c(number, vertices, type);
            ++counter;
            aModel.ct(type);
            dimension = starkeytodimension(starkey);
            if (dimension > -1)
                aModel.global_cellTypes[type]->dimension = dimension;
            cFile.read(&token1[0], 15);
        }
    }
    return counter;
}

size_t starIO::vwrite(const std::set<size_t>& vset, const std::string& filename) const {
    std::ofstream vFile(filename); // format for .vrt files: I9, 6X, 3G16.9
    size_t counter = 0;
    if (vFile.is_open()) {
#if defined (_MSC_VER)
        _set_output_format(_TWO_DIGIT_EXPONENT); // set digits after exponent to two. It is on VS, already default on gcc.
#endif
        std::set<size_t> valid_vset = aModel.validated_vset(vset);

        vFile << std::fixed;
        for (std::set<size_t>::const_iterator i = valid_vset.cbegin(); i != valid_vset.cend(); ++i, ++counter) {
            vFile << std::setw(9) << *i + offset_v << "      ";
            vFile << std::uppercase << std::setw(16) << std::setprecision(9) << aModel.global_vertices[*i]->x;
            vFile << std::uppercase << std::setw(16) << std::setprecision(9) << aModel.global_vertices[*i]->y;
            vFile << std::uppercase << std::setw(16) << std::setprecision(9) << aModel.global_vertices[*i]->z << std::endl;
        }
    }
    return counter;
}

size_t starIO::cwrite(const std::set<size_t>& cset, const std::string& filename) const {
    std::ofstream cFile(filename); // format for .cel files: I9, 6X, 9I9, 1X, I4
    // for cells with more than 8 vertices:  (I9, 6X, 9I9, 1X, I4)*3.
    size_t counter = 0;
    if (cFile.is_open()) {
        size_t VCount = 0;
        std::set<size_t> valid_cset = aModel.validated_cset(cset);
        short dimension = -1, starKey = 0; 
        for (std::set<size_t>::const_iterator i = valid_cset.cbegin(); i != valid_cset.cend(); ++i, ++counter) {
            cFile << std::setw(9) << *i + offset_c << "      ";
            VCount = aModel.global_cells[*i]->vertices.size() < 8 ? aModel.global_cells[*i]->vertices.size() : 8;
            for (int j = 0; j < VCount; ++j)
                cFile << std::setw(9) << aModel.global_cells[*i]->vertices[j] + offset_v;
            for (int j = VCount; j < 8; ++j)
                cFile << std::setw(9) << 0;
            cFile << std::setw(9) << aModel.global_cells[*i]->type << " ";
            if (aModel.global_cells[*i]->vertices.size() <= 8) {
                starKey = 0;
                if (aModel.global_cells[*i]->type < aModel.global_cellTypes.size() && aModel.global_cellTypes[aModel.global_cells[*i]->type]) 
                    starKey = dimensiontostarkey(aModel.global_cellTypes[aModel.global_cells[*i]->type]->dimension);
                cFile << std::setw(4) << starKey << std::endl;
            }
            else if (aModel.global_cells[*i]->vertices.size() > 8) {
                cFile << "  -2\n" << std::setw(9) << *i << "      ";
                VCount = aModel.global_cells[*i]->vertices.size() < 16 ? aModel.global_cells[*i]->vertices.size() : 16;
                for (int j = 8; j < VCount; ++j)
                    cFile << std::setw(9) << aModel.global_cells[*i]->vertices[j] + offset_v;;
                for (int j = VCount; j < 16; ++j)
                    cFile << std::setw(9) << 0;
                cFile << "       -2    0\n" << std::setw(9) << *i << "      ";
                VCount = aModel.global_cells[*i]->vertices.size() < 20 ? aModel.global_cells[*i]->vertices.size() : 20;
                for (int j = 16; j < VCount; ++j)
                    cFile << std::setw(9) << aModel.global_cells[*i]->vertices[j] + offset_v;
                for (int j = VCount; j < 23; ++j)
                    cFile << std::setw(9) << 0;
                starKey = 0;
                if (aModel.global_cells[*i]->type < aModel.global_cellTypes.size() && aModel.global_cellTypes[aModel.global_cells[*i]->type]) 
                    starKey = dimensiontostarkey(aModel.global_cellTypes[aModel.global_cells[*i]->type]->dimension);
                cFile << std::setw(9) << starKey << "       -2    0" << std::endl;
            }
        }
    }
    return counter;
}

std::pair<size_t, size_t> starIO::read(const std::string& vfilename, const std::string& cfilename) {
    size_t vcounter = 0, ccounter = 0;
    std::thread t1(&starIO::threaded_vread, this, vfilename, std::ref(vcounter));
    std::thread t2(&starIO::threaded_cread, this, cfilename, std::ref(ccounter));
    t1.join();
    t2.join();
    return std::pair<size_t, size_t>(vcounter, ccounter);
}

std::pair<size_t, size_t> starIO::write(const std::set<size_t> &vset, const std::string& vfilename, const std::set<size_t> &cset, const std::string& cfilename) const {
    size_t vcounter = 0, ccounter = 0;
    std::thread t1(&starIO::threaded_vwrite, this, std::ref(vset), vfilename, std::ref(vcounter));
    std::thread t2(&starIO::threaded_cwrite, this, std::ref(cset), cfilename, std::ref(ccounter));
    t1.join();
    t2.join();
    return std::pair<size_t, size_t>(vcounter, ccounter);
}

size_t starIO::vset_ifile(const std::set<size_t>& vset) {
    std::ofstream ifile(ifilename, mode);
    size_t vcounter = 0;
    if (ifile.is_open()) {
        for (std::set<size_t>::const_iterator i = vset.cbegin(); i != vset.cend(); ++i, ++vcounter) {
            if (vcounter == 0)
                ifile << "vset, newset, vlist";
            else if (vcounter % 15 == 0)
                ifile << std::endl << "vset, add, vlist";
            ifile << ", " << *i + offset_v;
        }
        ifile << std::endl;
        mode = std::ios_base::app;
    }
    return vcounter;
}

size_t starIO::cset_ifile(const std::set<size_t>& cset) {
    std::ofstream ifile(ifilename, mode);
    size_t ccounter = 0;
    if (ifile.is_open()) {
        for (std::set<size_t>::const_iterator i = cset.cbegin(); i != cset.cend(); ++i, ++ccounter) {
            if (ccounter == 0)
                ifile << "cset, newset, clist";
            else if (ccounter % 15 == 0)
                ifile << std::endl << "cset, add, clist";
            ifile << ", " << *i + offset_c;
        }
        ifile << std::endl;
        mode = std::ios_base::app;
    }
    return ccounter;
}

void starIO::vdel_ifile() {
    std::ofstream ifile(ifilename, mode);
    if (ifile.is_open()) {
        ifile << "vdel, cset" << std::endl;
        mode = std::ios_base::app;
    }
}

void starIO::cdel_ifile() {
    std::ofstream ifile(ifilename, mode);
    if (ifile.is_open()) {
        ifile << "cdel, cset" << std::endl;
        mode = std::ios_base::app;
    }
}

size_t ansysIO::vread(const std::string& filename) {
    std::ifstream vFile(filename); // format for .node files: I8, 1X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7
    size_t counter = 0;
    if (vFile.is_open()) {
        std::string token1(100, 0), token2(100, 0);
        size_t number = 0;
        double x = 0, y = 0, z = 0;
        while (vFile.good()) {
            vFile.read(&token1[0], 8);
            while (vFile.good()) {
                number = strtoul(&token1[0], NULL, 10) + offset_v;
                vFile.ignore(1);
                vFile.read(&token2[0], 14);
                x = strtod(&token2[0], NULL);
                vFile.ignore(6);
                vFile.read(&token2[0], 14);
                y = strtod(&token2[0], NULL);
                vFile.ignore(6);
                vFile.read(&token2[0], 14);
                z = strtod(&token2[0], NULL);
                aModel.v(number, x, y, z, 0);
                ++counter;
                vFile.ignore(61);
                vFile.read(&token1[0], 8);
            }
        }
    }
    return counter;
}

size_t ansysIO::cread(const std::string& filename) {
    std::ifstream cFile(filename); // format for .elem files: 14I8, for cells with more than 8 vertices, followed by a line with 12I8 
    size_t counter = 0;
    if (cFile.is_open()) {
        std::string token1(100, 0), token2(100, 0), token3(100, 0);
        size_t number = 0, type = 0, vNumber = 0;
        std::vector<size_t> vertices;
        short starkey = 0, dimension = -1;
        unsigned long long sectionNumber = 0, mType = 0, realConstant = 0, cellCSys = 0;
        cFile.read(&token1[0], 8);
        while (cFile.good()) {
            vertices.clear();
            for (int i = 0; i < 8; ++i) {
                vNumber = strtoul(&token1[0], NULL, 10);
                if (vNumber)
                    vertices.push_back(vNumber + offset_v);
                cFile.read(&token1[0], 8);
            }
            mType = strtoul(&token1[0], NULL, 10);
            cFile.read(&token1[0], 8);
            type = strtoul(&token1[0], NULL, 10);
            cFile.read(&token1[0], 8);
            realConstant = strtoul(&token1[0], NULL, 10);
            cFile.read(&token1[0], 8);
            sectionNumber = strtoul(&token1[0], NULL, 10);
            if (cFile.peek() == '\n') { // If true, this line is a second line of two-line cell definition
                for (int i = 0; i < vertices.size(); ++i)
                    aModel.global_cells[number]->vertices.push_back(vertices[i]);
                if (mType)
                    aModel.global_cells[number]->vertices.push_back(mType);
                if (type)
                    aModel.global_cells[number]->vertices.push_back(type);
                if (realConstant)
                    aModel.global_cells[number]->vertices.push_back(realConstant);
                if (sectionNumber)
                    aModel.global_cells[number]->vertices.push_back(sectionNumber);
            }
            else {
                cFile.read(&token1[0], 8);
                cellCSys = strtoul(&token1[0], NULL, 10);
                cFile.read(&token1[0], 8);
                number = strtoul(&token1[0], NULL, 10) + offset_c;
                aModel.c(number, vertices, type);
                ++counter;
                aModel.ct(type);
                aModel.global_cellTypes[type]->mType = mType;
                aModel.global_cellTypes[type]->realConstant = realConstant;
                aModel.global_cellTypes[type]->cellCSys = cellCSys;
            }
            cFile.ignore(1);
            cFile.read(&token1[0], 8);
        }
    }
    return counter;
}

size_t ansysIO::vwrite(const std::set<size_t>& vset, const std::string& filename) const {
    std::ofstream vFile(filename); // format for .node files: I8, 1X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7, 6X, G14.7
    size_t counter = 0;
    if (vFile.is_open()) {
#if defined (_MSC_VER)
        _set_output_format(_TWO_DIGIT_EXPONENT); // set digits after exponent to two. It is on VS, already default on gcc.
#endif
        std::set<size_t> valid_vset = aModel.validated_vset(vset);
        for (std::set<size_t>::const_iterator i = vset.cbegin(); i != vset.cend(); ++i, ++counter) {
            vFile << std::setw(8) << *i + offset_v << ' ' << std::setprecision(7) << std::uppercase;
            vFile << std::setw(14) << aModel.global_vertices[*i]->x << "      ";
            vFile << std::setw(14) << aModel.global_vertices[*i]->y << "      ";
            vFile << std::setw(14) << aModel.global_vertices[*i]->z << "      ";
            vFile << std::setw(14) << 0 << "      " << std::setw(14) << 0 << "      " << std::setw(14) << 0 << std::endl;
        }
    }
    return counter;
}

size_t ansysIO::cwrite(const std::set<size_t>& cset, const std::string& filename) const {
    std::ofstream cFile(filename); // format for .elem files: 14I8, for cells with more than 8 vertices, followed by a line with 12I8 
    size_t counter = 0;
    if (cFile.is_open()) {
        size_t VCount = 0;
        std::set<size_t> valid_cset = aModel.validated_cset(cset);
        for (std::set<size_t>::const_iterator i = valid_cset.cbegin(); i != valid_cset.cend(); ++i, ++counter) {
            VCount = aModel.global_cells[*i]->vertices.size() < 8 ? aModel.global_cells[*i]->vertices.size() : 8;
            for (size_t j = 0; j < VCount; ++j)
                cFile << std::setw(8) << aModel.global_cells[*i]->vertices[j] + offset_v;
            for (size_t j = VCount; j < 8; ++j)
                cFile << std::setw(8) << 0;
            if (aModel.global_cells[*i]->type < aModel.global_cellTypes.size() && aModel.global_cellTypes[aModel.global_cells[*i]->type]) {
                cFile << std::setw(8) << aModel.global_cellTypes[aModel.global_cells[*i]->type]->mType;
                cFile << std::setw(8) << aModel.global_cells[*i]->type;
                cFile << std::setw(8) << aModel.global_cellTypes[aModel.global_cells[*i]->type]->realConstant;
                cFile << std::setw(8) << 0; // Beam Section Number
                cFile << std::setw(8) << aModel.global_cellTypes[aModel.global_cells[*i]->type]->cellCSys;
            } 
            else
                cFile << std::setw(8) << 0 << std::setw(8) << aModel.global_cells[*i]->type << std::setw(8) << 0 << std::setw(8) << 0 << std::setw(8) << 0;
            cFile << std::setw(8) << *i + offset_c << std::endl;
            if (aModel.global_cells[*i]->vertices.size() > 8) {
                VCount = aModel.global_cells[*i]->vertices.size() < 20 ? aModel.global_cells[*i]->vertices.size() : 20;
                for (size_t j = 8; j < VCount; ++j)
                    cFile << std::setw(8) << aModel.global_cells[*i]->vertices[j] + offset_v;
                for (size_t j = VCount; j < 20; ++j)
                    cFile << std::setw(8) << 0;
                cFile << std::endl;
            }
        }
    }
    return counter;
}

std::pair<size_t, size_t> ansysIO::read(const std::string& vfilename, const std::string& cfilename) {
    size_t vcounter = 0, ccounter = 0;
    std::thread t1(&ansysIO::threaded_vread, this, vfilename, std::ref(vcounter));
    std::thread t2(&ansysIO::threaded_cread, this, cfilename, std::ref(ccounter));
    t1.join();
    t2.join();
    return std::pair<size_t, size_t>(vcounter, ccounter);
}

std::pair<size_t, size_t> ansysIO::write(const std::set<size_t> &vset, const std::string& vfilename, const std::set<size_t> &cset, const std::string& cfilename) const {
    size_t vcounter = 0, ccounter = 0;
    std::thread t1(&ansysIO::threaded_vwrite, this, std::ref(vset), vfilename, std::ref(vcounter));
    std::thread t2(&ansysIO::threaded_cwrite, this, std::ref(cset), cfilename, std::ref(ccounter));
    t1.join();
    t2.join();
    return std::pair<size_t, size_t>(vcounter, ccounter);
}

size_t ansysIO::vset_ifile(const std::set<size_t>& vset) {
    std::ofstream ifile(ifilename, mode);
    size_t vcounter = 0;
    if (ifile.is_open()) {
        for (std::set<size_t>::const_iterator i = vset.cbegin(); i != vset.cend(); ++i, ++vcounter) {
            if (vcounter == 0)
                ifile << "nsel, s, node, ";
            else
                ifile << std::endl << "nsel, a, node, ";
            ifile << ", " << *i + offset_v;
        }
        ifile << std::endl;
        mode = std::ios_base::app;
    }
    return vcounter;
}

size_t ansysIO::cset_ifile(const std::set<size_t>& cset) {
    std::ofstream ifile(ifilename, mode);
    size_t ccounter = 0;
    if (ifile.is_open()) {
        for (std::set<size_t>::const_iterator i = cset.cbegin(); i != cset.cend(); ++i, ++ccounter) {
            if (ccounter == 0)
                ifile << "esel, s, elem, ";
            else
                ifile << std::endl << "esel, a, elem, ";
            ifile << ", " << *i + offset_c;
        }
        ifile << std::endl;
        mode = std::ios_base::app;
    }
    return ccounter;
}

void ansysIO::vdel_ifile() {
    std::ofstream ifile(ifilename, mode);
    if (ifile.is_open()) {
        ifile << "ndele, all" << std::endl;
        mode = std::ios_base::app;
    }
}

void ansysIO::cdel_ifile() {
    std::ofstream ifile(ifilename, mode);
    if (ifile.is_open()) {
        ifile << "edele, all" << std::endl;
        mode = std::ios_base::app;
    }
}

std::pair<size_t, size_t> abaqusIO::read(const std::string& filename) {
    std::ifstream iFile(filename); // format for .inp files for nodes (after a line starting with *NODE): I, 3G (unformatted, seperated by comma)
    // format for .inp files for elements (after a line starting with *ELEMENT): I, I...I (unformatted, seperated by comma, can span multiple lines until a tokenends without comma)
    size_t vcounter = 0, ccounter = 0;
    if (iFile.is_open()) {
        std::string token;
        std::vector<std::string> tokens;
        std::vector<size_t> vertices;
        std::string partName = "";
        size_t vtype = aModel.apaxvt();
        size_t ctype = aModel.apaxct();
        cellABAQUS_t type = undefined_ABAQUS;
        while (iFile.good()) {
            getline(iFile, token);
            std::transform(token.begin(), token.end(), token.begin(), toupper);
            if (token.substr(0, 5) == "*NODE") {
                aModel.vt(vtype);
                tokens.clear();
                while (iFile.good() && iFile.peek() != '*') {
                    iFile >> token;
                    tokens.push_back(token);
                    if (token.back() != ',' && tokens.size() == 4) {
                        aModel.v(strtoul(&tokens[0][0], NULL, 10) + offset_v, strtod(&tokens[1][0], NULL), strtod(&tokens[2][0], NULL), strtod(&tokens[3][0], NULL), vtype);
                        ++vcounter;
                        tokens.clear();
                        iFile.ignore();
                    }
                }
            }
            if (token.substr(0, 8) == "*ELEMENT") {
                size_t type_start = token.find("TYPE=");
                std::string typeString;
                if (type_start != std::string::npos)
                    for (size_t i = type_start + 5; i < token.size() && token[i] != ' ' && token[i] != ','; ++i)
                        typeString.push_back(token[i]);
                if (typeString == "C3D8")
                    type = C3D8;
                else if (typeString == "C3D8R")
                    type = C3D8R;
                aModel.ct(ctype);
                aModel.global_cellTypes[ctype]->abaqusType = type;
                if (partName != "")
                    aModel.global_cellTypes[ctype]->name = partName;
                tokens.clear();
                while (iFile.good() && iFile.peek()!= '*') {
                    iFile >> token;
                    tokens.push_back(token);
                    if (token.back() != ',' && tokens.size() >= 2) {
                        vertices.clear();
                        for (size_t i = 1; i < tokens.size(); ++i)
                            vertices.push_back(strtoul(&tokens[i][0], NULL, 10) + offset_v);
                        aModel.c(strtoul(&tokens[0][0], NULL, 10) + offset_c, vertices, ctype);
                        ++ccounter;
                        tokens.clear();
                        iFile.ignore();
                    }
                }
            }
            if (token.substr(0, 5) == "*PART") {
                size_t name_start = token.find("NAME=");
                partName = "";
                if (name_start != std::string::npos)
                    for (size_t i = name_start + 5; i < token.size() && token[i] != ' ' && token[i] != ','; ++i)
                        partName.push_back(token[i]);
                if (vcounter > 0 || ccounter > 0) { 
                    ++vtype;
                    ++ctype;
                }
            }
        }
    }
    return std::pair<size_t, size_t>(vcounter, ccounter);
}

std::pair<size_t, size_t> abaqusIO::write(const std::set<size_t> &vset, const std::set<size_t> &cset, const std::string& filename) const {
    std::ofstream oFile(filename); // format for .inp files for nodes (after a line starting with *NODE): I, 3G (unformatted, seperated by comma)
    // format for .inp files for elements (after a line starting with *ELEMENT): I, I...I (unformatted, seperated by comma, can span multiple lines until a tokenends without comma)
    size_t vcounter = 0, ccounter = 0;
    if (oFile.is_open()) {
        for (size_t i = 0; i < aModel.global_cellTypes.size(); ++i) {
            if (aModel.global_cellTypes[i]) {
                std::set<size_t> cset = aModel.cnt(i);
                if (cset.size() == 1)
                    continue;
                std::set<size_t> vset;
                for (std::set<size_t>::iterator c = cset.begin(); c != cset.end(); ++c)
                    vset.insert(aModel.global_cells[*c]->vertices.begin(), aModel.global_cells[*c]->vertices.end());
                if (aModel.global_cellTypes[i]->name != "")
                    oFile << "*Part, name=" << aModel.global_cellTypes[i]->name << std::endl;
                else
                    oFile << "*Part, name=PART" << i << std::endl;
                oFile << "*Node" << std::endl;
                for (std::set<size_t>::iterator v = vset.begin(); v != vset.end(); ++v) {
                    if (aModel.global_vertices[*v]) {
                        oFile << *v + offset_v << ", " << std::setprecision(10) << aModel.global_vertices[*v]->x << ", " << aModel.global_vertices[*v]->y << ", " << aModel.global_vertices[*v]->z << std::endl;
                        ++vcounter;
                    }
                }
                oFile << "*Element";
                if (aModel.global_cellTypes[i]->abaqusType == C3D8)
                    oFile << ", type=C3D8";
                if (aModel.global_cellTypes[i]->abaqusType == C3D8R)
                    oFile << ", type=C3D8R";
                oFile << std::endl;
                for (std::set<size_t>::iterator c = cset.begin(); c != cset.end(); ++c) {
                    oFile << *c + offset_c << ", ";
                    for (size_t k = 0; k < aModel.global_cells[*c]->vertices.size() - 1; ++k) {
                        oFile << aModel.global_cells[*c]->vertices[k] + offset_v << ", ";
                        if ((k + 2) % 10 == 0)
                            oFile << '\n';
                    }
                    oFile << aModel.global_cells[*c]->vertices.back() << std::endl;
                    ++ccounter;
                }
            }
        }
    }
    return std::pair<size_t, size_t>(vcounter, ccounter);
}


