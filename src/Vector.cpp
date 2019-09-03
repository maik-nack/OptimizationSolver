#include <cstdlib>
#include <cmath>
#include <iostream>
#include <cstring>
#include "IVector.h"
#include "ILog.h"

#define DIM_CHECK(vector, funcName)\
{if (size != vector->getDim())\
{ char msg[100] = "IVector.";\
    strcat(msg, funcName);\
    strcat(msg, ": Vectors dimensions mismatch.\n");\
    ILog::report(msg);\
    return ERR_DIMENSIONS_MISMATCH; }};

#define RANGE_CHECK(ind, funcName)\
{if (ind >= size)\
{ char msg[100] = "IVector.";\
    strcat(msg, funcName);\
    strcat(msg, ": Index out of range.\n");\
    ILog::report(msg);\
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
    Vector(unsigned int size, double* vals);

    /*dtor*/
    ~Vector();

private:
    double* vals;
    size_t size;

    /*non default copyable*/
    Vector(const IVector& other) = delete;
    void operator=(const Vector& other) = delete;
};
}

Vector::Vector(unsigned int size, double *vals)
{
    this->size = size;
    this->vals = vals;
}

Vector::~Vector()
{
    delete[] vals;
}

IVector* IVector::createVector(unsigned int size, double const* vals)
{
    if (!vals)
    {
        ILog::report("IVector.createVector: Can't create vector, vals - nullptr.\n");
        return NULL;
    }

    double *valsCopy = new(std::nothrow) double[size];
    if (!valsCopy)
    {
        ILog::report("IVector.createVector: Not enough memory.\n");
        return NULL;
    }

    for (unsigned int i = 0; i < size; ++i)
        valsCopy[i] = vals[i];

    IVector *v = new(std::nothrow) Vector(size, valsCopy);
    if (!v)
    {
        delete[] valsCopy;
        ILog::report("IVector.createVector: Not enough memory.\n");
        return NULL;
    }

    return v;
}

int Vector::add(IVector const* const right)
{
    if (!right)
    {
        ILog::report("IVector.add: Can't add vector, right - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right, "add");

    int errCode;
    double coord;
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("IVector.add: Not enough memory.\n");
        return ERR_MEMORY_ALLOCATION;
    }

    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if (errCode != ERR_OK)
        {
            delete[] vals;
            return errCode;
        }
        vals[i] = this->vals[i] + coord;
    }

    delete[] this->vals;
    this->vals = vals;

    return ERR_OK;
}

int Vector::subtract(IVector const* const right)
{
    if (!right)
    {
        ILog::report("IVector.subtract: Can't subtract vector, right - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right, "subtract");

    int errCode;
    double coord;
    double *vals = new(std::nothrow) double[size];
    if (!vals)
    {
        ILog::report("IVector.subtract: Not enough memory.\n");
        return ERR_MEMORY_ALLOCATION;
    }

    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if (errCode != ERR_OK)
        {
            delete[] vals;
            return errCode;
        }
        vals[i] = this->vals[i] - coord;
    }

    delete[] this->vals;
    this->vals = vals;

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
        ILog::report("IVector.dotProduct: Can't calculate dot product, right vector - nullptr.\n");
        return ERR_WRONG_ARG;
    }
    DIM_CHECK(right, "dotProduct");

    int errCode;
    double coord;

    for (unsigned int i = 0; i < size; ++i)
    {
        errCode = right->getCoord(i, coord);
        if (errCode != ERR_OK)
            return errCode;

        res += vals[i] * coord;
    }

    return ERR_OK;
}

IVector* IVector::add(IVector const* const left, IVector const* const right)
{
    if (!right || !left || left->getDim() != right->getDim())
    {
        ILog::report("IVector.add: Can't add vector, some of vectors - nullptr or dimensions mismatch.\n");
        return NULL;
    }

    IVector *v = left->clone();
    if (!v)
    {
        ILog::report("IVector.add: Can't add vector, can't clone vector.\n");
        return NULL;
    }
    int errCode = v->add(right);
    if(errCode != ERR_OK)
    {
        delete v;
        return NULL;
    }

    return v;
}

IVector* IVector::subtract(IVector const* const left, IVector const* const right)
{
    if (!right || !left || left->getDim() != right->getDim())
    {
        ILog::report("IVector.subtract: Can't subtract vector, some of vectors - nullptr or dimensions mismatch.\n");
        return NULL;
    }

    IVector *v = left->clone();
    if (!v)
    {
        ILog::report("IVector.subtract: Can't subtract vector, can't clone vector.\n");
        return NULL;
    }
    int errCode = v->subtract(right);
    if(errCode != ERR_OK)
    {
        delete v;
        return NULL;
    }

    return v;
}

IVector* IVector::multiplyByScalar(IVector const* const left, double scalar)
{
    if (!left)
    {
        ILog::report("IVector.multiplyByScalar: Can't multiply vector by scalar, vector - nullpt.\n");
        return NULL;
    }

    IVector *v = left->clone();
    if (!v)
    {
        ILog::report("IVector.multiplyByScalar: Can't multiply vector by scalar, can't clone vector.\n");
        return NULL;
    }
    int errCode = v->multiplyByScalar(scalar);
    if(errCode != ERR_OK)
    {
        delete v;
        return NULL;
    }

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
        ILog::report("IVector.norm: Norm not defined.\n");
        return ERR_NORM_NOT_DEFINED;
    }
    return ERR_OK;
}

int Vector::setCoord(unsigned int index, double elem)
{
    RANGE_CHECK(index, "setCoord");
    vals[index] = elem;

    return ERR_OK;
}

int Vector::getCoord(unsigned int index, double & elem) const
{
    RANGE_CHECK(index, "getCoord");
    elem = vals[index];

    return ERR_OK;
}

int Vector::setAllCoords(unsigned int dim, double* coords)
{
    if (dim != size)
    {
        ILog::report("IVector.setAllCoords: Dimensions mismatch.\n");
        return ERR_DIMENSIONS_MISMATCH;
    }

    if (!coords)
    {
        ILog::report("IVector.setAllCoords: Coords - nullptr.\n");
        return ERR_WRONG_ARG;
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
    return createVector(size, vals);
}

int Vector::gt(IVector const* const right, NormType type, bool& result) const
{
    if (!right)
    {
        ILog::report("IVector.gt: Right - nullptr.\n");
        return ERR_WRONG_ARG;
    }

    double norm, normR;
    int errCode;

    errCode = this->norm(type, norm);
    if (errCode != ERR_OK)
        return errCode;
    errCode = right->norm(type, normR);
    if (errCode != ERR_OK)
        return errCode;

    result = norm > normR;

    return ERR_OK;
}

int Vector::lt(IVector const* const right, NormType type, bool& result) const
{
    if (!right)
    {
        ILog::report("IVector.lt: Right - nullptr.\n");
        return ERR_WRONG_ARG;
    }

    double norm, normR;
    int errCode;

    errCode = this->norm(type, norm);
    if (errCode != ERR_OK)
        return errCode;
    errCode = right->norm(type, normR);
    if (errCode != ERR_OK)
        return errCode;

    result = norm < normR;

    return ERR_OK;
}

int Vector::eq(IVector const* const right, NormType type, bool& result, double precision) const
{
    if (!right)
    {
        ILog::report("IVector.eq: Right - nullptr.\n");
        return ERR_WRONG_ARG;
    }

    IVector *v = IVector::subtract(this, right);
    if (!v)
    {
        ILog::report("IVector.eq: Can't subtract vectors.\n");
        return ERR_WRONG_ARG;
    }

    double norm;
    int errCode = v->norm(type, norm);
    delete v;
    if (errCode != ERR_OK)
        return errCode;

    result = norm < precision;

    return ERR_OK;
}

int Vector::getId() const
{
    return IVector::INTERFACE_0;
}
