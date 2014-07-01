/*
* File:   IO.h
* Author: Saeid Emami
*
* Created on June 25, 2013, 11:49 PM
*/

#ifndef IO_H
#define IO_H

#include "Mesh.h"

enum IO_t{STAR, ANSYS, ABAQUS, OBJECT, UNDEFINED};

class tokenizer {
private:
    std::string tokenline;
    size_t tokencount;
    std::string _str();
public:
    bool flag;
    tokenizer(const std::string &line): tokenline(line), tokencount(0), flag(false) {};
    std::string str();
    size_t UL();
    double D();
    IO_t IOt();
};

class starIO {
private:
    TModel& aModel;
    std::ios_base::openmode mode;
    short dimensiontostarkey(const short dimension) const;
    short starkeytodimension(const short starkey) const;
    void threaded_vread(const std::string& filename, size_t& result) {result = this->vread(filename);};
    void threaded_cread(const std::string& filename, size_t& result) {result = this->cread(filename);};
    void threaded_vwrite(const std::set<size_t> &vset, const std::string& filename, size_t& result) const {result = this->vwrite(vset, filename);};
    void threaded_cwrite(const std::set<size_t> &cset, const std::string& filename, size_t& result) const {result = this->cwrite(cset, filename);};
public:
    long offset_v, offset_c;
    std::string ifilename;
    starIO(TModel& myModel): offset_v(0), offset_c(0), aModel(myModel), mode(std::ios_base::trunc) {};

    size_t vread(const std::string& filename);
    size_t cread(const std::string& filename);
    size_t vwrite(const std::set<size_t>& vset, const std::string& filename) const;
    size_t cwrite(const std::set<size_t>& cset, const std::string& filename) const;
    std::pair<size_t, size_t> read(const std::string& vfilename, const std::string& cfilename);
    std::pair<size_t, size_t> write(const std::set<size_t> &vset, const std::string& vfilename, const std::set<size_t> &cset, const std::string& cfilename) const;

    size_t vset_ifile(const std::set<size_t>& vset);
    size_t cset_ifile(const std::set<size_t>& cset);
    void vdel_ifile();
    void cdel_ifile();
};

class ansysIO {
private:
    TModel& aModel;
    std::ios_base::openmode mode;
    void threaded_vread(const std::string& filename, size_t& result) {result = this->vread(filename);};
    void threaded_cread(const std::string& filename, size_t& result) {result = this->cread(filename);};
    void threaded_vwrite(const std::set<size_t> &vset, const std::string& filename, size_t& result) const {result = this->vwrite(vset, filename);};
    void threaded_cwrite(const std::set<size_t> &cset, const std::string& filename, size_t& result) const {result = this->cwrite(cset, filename);};
public:
    long offset_v, offset_c;
    std::string ifilename;
    ansysIO(TModel& myModel): offset_v(0), offset_c(0), aModel(myModel), mode(std::ios_base::trunc) {};

    size_t vread(const std::string& filename);
    size_t cread(const std::string& filename);
    size_t vwrite(const std::set<size_t>& vset, const std::string& filename) const;
    size_t cwrite(const std::set<size_t>& cset, const std::string& filename) const;
    std::pair<size_t, size_t> read(const std::string& vfilename, const std::string& cfilename);
    std::pair<size_t, size_t> write(const std::set<size_t> &vset, const std::string& vfilename, const std::set<size_t> &cset, const std::string& cfilename) const;

    size_t vset_ifile(const std::set<size_t>& vset);
    size_t cset_ifile(const std::set<size_t>& cset);
    void vdel_ifile();
    void cdel_ifile();
};

class abaqusIO {
private:
    TModel& aModel;
public:
    long offset_v, offset_c;
    std::string ifilename;
    abaqusIO(TModel& myModel): offset_v(0), offset_c(0), aModel(myModel) {};

    std::pair<size_t, size_t> read(const std::string& filename);
    std::pair<size_t, size_t> write(const std::set<size_t> &vset, const std::set<size_t> &cset, const std::string& filename) const;
};




#endif	/* IO_H */
