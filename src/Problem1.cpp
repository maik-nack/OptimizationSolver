#include <new>

#include "ILog.h"
#include "IBrocker.h"
#include "IProblem.h"
#include "IVector.h"

namespace {

class Problem1 : public IProblem {

public:

    int getId() const;

    int goalFunction(IVector const* args, IVector const* params, double& res) const;
    int goalFunctionByArgs(IVector const*  args, double& res) const;
    int goalFunctionByParams(IVector const*  params, double& res) const;
    int getArgsDim(size_t& dim) const;
    int getParamsDim(size_t& dim) const;

    int setParams(IVector const* params);
    int setArgs(IVector const* args);

    int derivativeGoalFunction(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args, IVector const* params) const;
    int derivativeGoalFunctionByArgs(size_t order, size_t idx, DerivedType dr, double& value, IVector const* args) const;
    int derivativeGoalFunctionByParams(size_t order, size_t idx, DerivedType dr, double& value, IVector const* params) const;

    /*ctor*/
    Problem1();

    /*dtor*/
    ~Problem1();

private:

    size_t _dimArgs, _dimParams;
    IVector *_args, *_params;

};

class Brocker1 : public IBrocker {

public:

    int getId() const;

    bool canCastTo(Type type) const;
    void* getInterfaceImpl(Type type) const;

    int release();

    /*ctor*/
    Brocker1(Problem1 *problem);

    /*dtor*/
    ~Brocker1();

private:

    Problem1 *_problem;

};

}

int Problem1::getId() const {
    return IProblem::INTERFACE_0;
}

int Problem1::goalFunction(IVector const* args,
                           IVector const* params, double& res) const {
    if (!args) {
        ILog::report("IProblem.goalFunction: Input argument args is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (!params) {
        ILog::report("IProblem.goalFunction: Input argument params is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (_dimArgs != args->getDim()) {
        ILog::report("IProblem.goalFunction: Input argument args has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    if (_dimParams != params->getDim()) {
        ILog::report("IProblem.goalFunction: Input argument params has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    unsigned int dimA, dimP;
    const double *a, *p;
    int ec;

    if ((ec = args->getCoordsPtr(dimA, a)) != ERR_OK)
        return ec;

    if ((ec = params->getCoordsPtr(dimP, p)) != ERR_OK)
        return ec;

    res = a[0] * a[0] + a[1] * a[1] +
            p[0] * p[0] - 4 * p[0] + p[1] * p[1] - 2 * p[1];

    return ERR_OK;
}

int Problem1::goalFunctionByArgs(IVector const*  args, double& res) const {
    return goalFunction(args, _params, res);
}

int Problem1::goalFunctionByParams(IVector const*  params, double& res) const {
    return goalFunction(_args, params, res);
}

int Problem1::getArgsDim(size_t& dim) const {
    dim = _dimArgs;

    return ERR_OK;
}

int Problem1::getParamsDim(size_t& dim) const {
    dim = _dimParams;

    return ERR_OK;
}

int Problem1::setParams(IVector const* params) {
    if (!params) {
        ILog::report("IProblem.setParams: Input argument params is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (_dimParams != params->getDim()) {
        ILog::report("IProblem.setParams: Input argument params has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    _params = params->clone();

    if (!_params) {
        ILog::report("IProblem.setParams: Not enough memory\n");
        return ERR_MEMORY_ALLOCATION;
    }

    return ERR_OK;
}

int Problem1::setArgs(IVector const* args) {
    if (!args) {
        ILog::report("IProblem.setArgs: Input argument args is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (_dimArgs != args->getDim()) {
        ILog::report("IProblem.setArgs: Input argument args has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    _args = args->clone();

    if (!_args) {
        ILog::report("IProblem.setArgs: Not enough memory\n");
        return ERR_MEMORY_ALLOCATION;
    }

    return ERR_OK;
}

int Problem1::derivativeGoalFunction(size_t order,
                                     size_t idx,
                                     DerivedType dr,
                                     double& value,
                                     IVector const* args,
                                     IVector const* params) const {
    if (!args) {
        ILog::report("IProblem.derivativeGoalFunction: Input argument args is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (!params) {
        ILog::report("IProblem.derivativeGoalFunction: Input argument params is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (_dimArgs != args->getDim()) {
        ILog::report("IProblem.derivativeGoalFunction: Input argument args has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    if (_dimParams != params->getDim()) {
        ILog::report("IProblem.derivativeGoalFunction: Input argument params has wrong dim\n");
        return ERR_VARIABLES_NUMBER_MISMATCH;
    }

    if (order == 0)
        return goalFunction(args, params, value);

    if (order == 2) {
        value = 2;
        return ERR_OK;
    }

    if (order >= 3) {
        value = 0;
        return ERR_OK;
    }

    int ec;

    switch (dr) {
    case IProblem::BY_ARGS:
        if (idx >= _dimArgs) {
            ILog::report("IProblem.derivativeGoalFunction: idx out of range\n");
            return ERR_OUT_OF_RANGE;
        }

        unsigned int dimA;
        const double *a;

        if ((ec = args->getCoordsPtr(dimA, a)) != ERR_OK)
            return ec;

        value = 2 * a[idx];

        return ERR_OK;

    case IProblem::BY_PARAMS:
        if (idx >= _dimParams) {
            ILog::report("IProblem.derivativeGoalFunction: idx out of range\n");
            return ERR_OUT_OF_RANGE;
        }

        unsigned int dimP;
        const double *p;

        if ((ec = params->getCoordsPtr(dimP, p)) != ERR_OK)
            return ec;

        if (idx == 0)
            value = 2 * p[0] - 4;
        else
            value = 2 * p[1] - 2;

        return ERR_OK;

    default:
        ILog::report("IProblem.derivativeGoalFunction: Input argument dr is unknown\n");
        return ERR_WRONG_ARG;
    }
}

int Problem1::derivativeGoalFunctionByArgs(size_t order,
                                           size_t idx,
                                           DerivedType dr,
                                           double& value,
                                           IVector const* args) const {
    return derivativeGoalFunction(order, idx, dr, value, args, _params);
}

int Problem1::derivativeGoalFunctionByParams(size_t order,
                                   size_t idx,
                                   DerivedType dr,
                                   double& value,
                                   IVector const* params) const {
    return derivativeGoalFunction(order, idx, dr, value, _args, params);
}

Problem1::Problem1():
    _dimArgs(2), _dimParams(2), _args(NULL), _params(NULL)
{}

Problem1::~Problem1() {
    delete _args;
    delete _params;
}

int Brocker1::getId() const {
    return IBrocker::INTERFACE_0;
}

bool Brocker1::canCastTo(Type type) const {
    switch (type) {
    case IBrocker::PROBLEM:
        return true;
    default:
        return false;
    }
}

void* Brocker1::getInterfaceImpl(Type type) const {
    switch (type) {
    case IBrocker::PROBLEM:
        return _problem;
    default:
        return NULL;
    }
}

int Brocker1::release() {
    delete this;

    return ERR_OK;
}

Brocker1::Brocker1(Problem1 *problem):
    _problem(problem)
{}

Brocker1::~Brocker1() {
    delete _problem;
}

extern "C" {
SHARED_EXPORT void* getBrocker() {
    Problem1 *problem = new (std::nothrow) Problem1();

    if (!problem) {
        ILog::report("getBrocker: not enough memory\n");
        return NULL;
    }

    Brocker1 *brocker = new (std::nothrow) Brocker1(problem);

    if (!brocker) {
        ILog::report("getBrocker: not enough memory\n");
        delete problem;
        return NULL;
    }

    return brocker;
}
}
