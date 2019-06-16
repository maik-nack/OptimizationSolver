//#include <QtGlobal>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include "IVector.h"
#include "ILog.h"

#define DIM_CHECK(vector)\
{if (size != vector->getDim())\
{ ILog::report("Vectors dimensions mismatch.\n");\
    return ERR_DIMENSIONS_MISMATCH; }};

#define RANGE_CHECK(ind)\
{if (ind >= size)\
{ ILog::report("Index out of range.\n");\
    return ERR_OUT_OF_RANGE; }};

namespace {

class Vector : public IVector {

public:

    int getId() const;

    /*operations*/
    int add(IVector const* const right);
    int subtract(IVector const* const right);
    int multiplyByScalar(double scalar);
    int dotProduct(IVector const* const right, double& res) const;
    int crossProduct(IVector const* const right);

    /*utils*/
    unsigned int getDim() const;
    int norm(NormType type, double& res) const;
    int setCoord(unsigned int index, double elem);
    int getCoord(unsigned int index, double & elem) const;
    int setAllCoords(unsigned int dim, double* coords);
    int getCoordsPtr(unsigned int & dim, double const*& elem) const;
    IVector* clone() const;

    /*comparators*/
    int gt(IVector const* const right, NormType type, bool& result) const;
    int lt(IVector const* const right, NormType type, bool& result) const;
    int eq(IVector const* const right, NormType type, bool& result, double precision) const;

    /*ctor*/
    Vector(unsigned int size, double const* vals);

    /*dtor*/
    ~Vector();

protected:
    Vector() = default;

private:
    double* vals;
    size_t size;

    /*non default copyable*/
    Vector(const IVector& other) = delete;
    void operator=(const Vector& other) = delete;
};
}

Vector::Vector(unsigned int size, const double *vals)
{
    this->size = size;
    this->vals = new(std::nothrow) double[size];
    for(unsigned int i = 0; i < size; ++i)
        this->vals[i] = vals[i];
}

Vector::~Vector()
{
    delete[] vals;
}

IVector* IVector::createVector(unsigned int size, double const* vals)
{
    if(!vals)
    {
        ILog::report("Can't create vector, vals - nullptr.\n");
        return NULL;
    }
    return new(std::nothrow) Vector(size, vals);
}

int Vector::add(IVector const* const right)
{
    if (!right)
    {
        ILog::report("Can't add vector, right - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right);

    int errCode;
    double coord;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if(errCode != ERR_OK)
            return errCode;

        vals[i] += coord;
    }

    return ERR_OK;
}

int Vector::subtract(IVector const* const right)
{
    if (!right)
    {
        ILog::report("Can't subtract vector, right - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right);

    int errCode;
    double coord;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if(errCode != ERR_OK)
            return errCode;

        vals[i] -= coord;
    }

    return ERR_OK;
}

int Vector::multiplyByScalar(double scalar)
{
    for (unsigned int i = 0; i < size; ++i)
        vals[i] *= scalar;

    return ERR_OK;
}

int Vector::dotProduct(IVector const* const right, double& res) const
{
    res = 0;
    if (!right)
    {
        ILog::report("Can't calculate dot product, right vector - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right);

    int errCode;
    double coord;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if(errCode != ERR_OK)
            return errCode;

        res += vals[i] * coord;
    }

    return ERR_OK;
}

int Vector::crossProduct(IVector const* const right)
{
    qt_assert("NOT IMPLEMENTED", __FILE__, __LINE__);
    return ERR_NOT_IMPLEMENTED;
}

IVector* IVector::add(IVector const* const left, IVector const* const right)
{
    if (!right || !left || left->getDim() != right->getDim())
    {
        ILog::report("Can't add vector, some of vectors - nullptr or dimensions mismatch.\n");
        return NULL;
    }

    double coordL, coordR;
    unsigned int size = left->getDim();
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("Not enough memory.\n");
        return NULL;
    }

    int errCode;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = left->getCoord(i, coordL);
        if(errCode != ERR_OK)
            return NULL;
        errCode = right->getCoord(i, coordR);
        if(errCode != ERR_OK)
            return NULL;

        vals[i] = coordL + coordR;
    }

    IVector *v = createVector(size, vals);

    delete[] vals;

    return v;
}

IVector* IVector::subtract(IVector const* const left, IVector const* const right)
{
    if (!right || !left || left->getDim() != right->getDim())
    {
        ILog::report("Can't subtract vector, some of vectors - nullptr or dimensions mismatch.\n");
        return NULL;
    }

    double coordL, coordR;
    unsigned int size = left->getDim();
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("Not enough memory.\n");
        return NULL;
    }

    int errCode;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = left->getCoord(i, coordL);
        if(errCode != ERR_OK)
            return NULL;
        errCode = right->getCoord(i, coordR);
        if(errCode != ERR_OK)
            return NULL;

        vals[i] = coordL - coordR;
    }

    IVector *v = createVector(size, vals);

    delete[] vals;

    return v;
}

IVector* IVector::multiplyByScalar(IVector const* const left, double scalar)
{
    if (!left)
    {
        ILog::report("Can't multiply vector by scalar, vector - nullpt.\n");
        return NULL;
    }

    double coord;
    unsigned int size = left->getDim();
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("Not enough memory.\n");
        return NULL;
    }

    int errCode;
    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = left->getCoord(i, coord);
        if(errCode != ERR_OK)
            return NULL;

        vals[i] = coord * scalar;
    }

    IVector *v = createVector(size, vals);

    delete[] vals;

    return v;
}

unsigned int Vector::getDim() const
{
    return size;
}

int Vector::norm(NormType type, double& res) const
{
    res = 0;
    switch(type)
    {
    case NORM_1:
        for (unsigned int i = 0; i < size; ++i)
            res += abs(vals[i]);
        break;

    case NORM_2:
        for (unsigned int i = 0; i < size; ++i)
            res += vals[i] * vals[i];
        res = sqrt(res);
        break;

    case NORM_INF:
        for (unsigned int i = 0; i < size; ++i)
        {
            if (abs(vals[i]) > res)
                res = abs(vals[i]);
        }
        break;

    default:
        ILog::report("Norm not defined.\n");
        return ERR_NORM_NOT_DEFINED;
    }
    return ERR_OK;
}

int Vector::setCoord(unsigned int index, double elem)
{
    RANGE_CHECK(index);
    vals[index] = elem;

    return ERR_OK;
}

int Vector::getCoord(unsigned int index, double & elem) const
{
    RANGE_CHECK(index);
    elem = vals[index];

    return ERR_OK;
}

int Vector::setAllCoords(unsigned int dim, double* coords)
{
    if(dim != size)
    {
        ILog::report("Dimensions mismatch.\n");
        return ERR_DIMENSIONS_MISMATCH;
    }

    for (unsigned int i = 0; i < size; ++i)
        vals[i] = coords[i];

    return ERR_OK;
}

int Vector::getCoordsPtr(unsigned int & dim, double const*& elem) const
{
    dim = size;
    elem = vals;

    return ERR_OK;
}

IVector* Vector::clone() const
{
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("Not enough memory.\n");
        return NULL;
    }

    for (unsigned int i = 0; i < size; ++i)
        vals[i] = this->vals[i];

    IVector *v = createVector(size, vals);

    delete[] vals;

    return v;
}

int Vector::gt(IVector const* const right, NormType type, bool& result) const
{
    double norm, normR;
    int errCode;

    errCode = this->norm(type, norm);
    if (errCode != ERR_OK)
        return errCode;
    errCode = right->norm(type, normR);
    if(errCode != ERR_OK)
        return errCode;

    result = norm > normR;

    return ERR_OK;
}

int Vector::lt(IVector const* const right, NormType type, bool& result) const
{
    double norm, normR;
    int errCode;

    errCode = this->norm(type, norm);
    if (errCode != ERR_OK)
        return errCode;
    errCode = right->norm(type, normR);
    if(errCode != ERR_OK)
        return errCode;

    result = norm < normR;

    return ERR_OK;
}

int Vector::eq(IVector const* const right, NormType type, bool& result, double precision) const
{
    double norm, normR;
    int errCode;

    errCode = this->norm(type, norm);
    if (errCode != ERR_OK)
        return errCode;
    errCode = right->norm(type, normR);
    if(errCode != ERR_OK)
        return errCode;

    result = abs(norm - normR) < precision;

    return ERR_OK;
}

int Vector::getId() const
{
    return INTERFACE_0;
}
