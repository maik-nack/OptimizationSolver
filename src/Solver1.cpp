#include <new>
#include <QFile>
#include <cmath>
#include <QRegExp>
#include <QString>
#include <QStringList>

#include "ILog.h"
#include "IBrocker.h"
#include "ISolver.h"
#include "IProblem.h"
#include "ICompact.h"

namespace {

class Solver1 : public ISolver {

public:

   int getId() const;

   int setParams(IVector const* params);
   int setParams(QString& str);
   int setProblem(IProblem *ptr);
   int solve();
   int getSolution(IVector* &vec) const;
   int getQml(QUrl& qml) const;

     /*ctor*/
   Solver1();

     /*dtor*/
   ~Solver1();

private:

   IVector * _args, * _params;
   ICompact * _compact;
   bool solveByArgs;
   double eps;
   IVector * _prev, * _curr;
   IProblem * _problem;

};

class Brocker2 : public IBrocker {

public:

   int getId() const;

   bool canCastTo(Type type) const;
   void* getInterfaceImpl(Type type) const;

   int release();

     /*ctor*/
   Brocker2(Solver1 *solver);

     /*dtor*/
   ~Brocker2();

private:

    Solver1 *_solver;

};

}


int Solver1::solve() {
    if (!_args || !_params) {
        ILog::report("ISolver.solve: initial approximation is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (!_compact) {
        ILog::report("ISolver.solve: compact is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if(!_problem) {
        ILog::report("ISolver.solve: problem is nullptr\n");
        return ERR_WRONG_ARG;
    }

    if (solveByArgs) {
        if(_problem->setParams(_params) != ERR_OK) {
            ILog::report("ISolver.solve: error with setting params to problem\n");
            return ERR_ANY_OTHER;
        }

        _curr = _args->clone();

        if (!_curr) {
            ILog::report("ISolver.solve: not enough memory\n");
            return ERR_MEMORY_ALLOCATION;
        }

        while (true) {
            size_t argsDim;

            if (_problem->getArgsDim(argsDim) != ERR_OK) {
                ILog::report("ISolver.solve: error with getArgsDim\n");
                return ERR_ANY_OTHER;
            }

            double *gradD = new (std::nothrow) double[argsDim];

            if (!gradD) {
                ILog::report("ISolver.solve: not enough memory\n");
                return ERR_MEMORY_ALLOCATION;
            }

            for (size_t i = 0; i < argsDim; i++) {
                if (_problem->derivativeGoalFunctionByArgs(1,
                                                           i,
                                                           IProblem::BY_ARGS,
                                                           gradD[i],
                                                           _curr) != ERR_OK) {
                    ILog::report("ISolver.solve: error with derivativeGoalFunctionByArgs\n");
                    delete[] gradD;
                    return ERR_ANY_OTHER;
                }
            }

            IVector *gradV = IVector::createVector(argsDim, gradD);

            if (!gradV) {
                ILog::report("ISolver.solve: not enough memory\n");
                delete[] gradD;
                return ERR_MEMORY_ALLOCATION;
            }

            delete[] gradD;

            double alpha = 1, lambda = 0.8;

            while (true) {
                IVector *tmpMS = IVector::multiplyByScalar(gradV, alpha);

                if (!tmpMS) {
                    ILog::report("ISolver.solve: error with multiplyByScalar\n");
                    delete gradV;
                    return ERR_ANY_OTHER;
                }

                IVector *tmpS = IVector::subtract(_curr, tmpMS);

                if (!tmpS) {
                    ILog::report("ISolver.solve: error with subtract\n");
                    delete gradV;
                    delete tmpMS;
                    return ERR_ANY_OTHER;
                }

                IVector *prS;

                if (_compact->getNearestNeighbor(tmpS, prS) != ERR_OK) {
                    ILog::report("ISolver.solve: error with getNearestNeighbor\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    return ERR_ANY_OTHER;
                }

                double resS, resC;

                if (_problem->goalFunctionByArgs(prS, resS) != ERR_OK) {
                    ILog::report("ISolver.solve: error with goalFunctionByArgs\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    delete prS;
                    return ERR_ANY_OTHER;
                }

                if (_problem->goalFunctionByArgs(_curr, resC) != ERR_OK) {
                    ILog::report("ISolver.solve: error with goalFunctionByArgs\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    delete prS;
                    return ERR_ANY_OTHER;
                }

                if (resS < resC) {
                    delete _prev;
                    _prev = _curr;
                    _curr = prS;
                    break;
                } else {
                    alpha *= lambda;
                }

                delete tmpMS;
                delete tmpS;
            }

            delete gradV;

            bool res = false;

            if (_curr->eq(_prev, IVector::NORM_INF, res, eps))
                break;
        }
    }
    else {
        if(_problem->setArgs(_args) != ERR_OK) {
            ILog::report("ISolver.solve: error with setting args to problem\n");
            return ERR_ANY_OTHER;
        }

        _curr = _args->clone();

        if (!_curr) {
            ILog::report("ISolver.solve: not enough memory\n");
            return ERR_MEMORY_ALLOCATION;
        }

        while (true) {
            size_t paramsDim;

            if (_problem->getParamsDim(paramsDim) != ERR_OK) {
                ILog::report("ISolver.solve: error with getParamsDim\n");
                return ERR_ANY_OTHER;
            }

            double *gradD = new (std::nothrow) double[paramsDim];

            if (!gradD) {
                ILog::report("ISolver.solve: not enough memory\n");
                return ERR_MEMORY_ALLOCATION;
            }

            for (size_t i = 0; i < paramsDim; i++) {
                if (_problem->derivativeGoalFunctionByParams(1,
                                                           i,
                                                           IProblem::BY_PARAMS,
                                                           gradD[i],
                                                           _params) != ERR_OK) {
                    ILog::report("ISolver.solve: error with derivativeGoalFunctionByParams\n");
                    delete[] gradD;
                    return ERR_ANY_OTHER;
                }
            }

            IVector *gradV = IVector::createVector(paramsDim, gradD);

            if (!gradV) {
                delete[] gradD;
                ILog::report("ISolver.solve: not enough memory\n");
                return ERR_MEMORY_ALLOCATION;
            }

            delete[] gradD;

            double alpha = 1, lambda = 0.8;

            while (true) {
                IVector *tmpMS = IVector::multiplyByScalar(gradV, alpha);

                if (!tmpMS) {
                    ILog::report("ISolver.solve: error with multiplyByScalar\n");
                    delete gradV;
                    return ERR_ANY_OTHER;
                }

                IVector *tmpS = IVector::subtract(_curr, tmpMS);

                if (!tmpS) {
                    ILog::report("ISolver.solve: error with subtract\n");
                    delete gradV;
                    delete tmpMS;
                    return ERR_ANY_OTHER;
                }

                IVector *prS;

                if (_compact->getNearestNeighbor(tmpS, prS) != ERR_OK) {
                    ILog::report("ISolver.solve: error with getNearestNeighbor\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    return ERR_ANY_OTHER;
                }

                double resS, resC;

                if (_problem->goalFunctionByParams(prS, resS) != ERR_OK) {
                    ILog::report("ISolver.solve: error with goalFunctionByArgs\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    delete prS;
                    return ERR_ANY_OTHER;
                }

                if (_problem->goalFunctionByParams(_curr, resC) != ERR_OK) {
                    ILog::report("ISolver.solve: error with goalFunctionByArgs\n");
                    delete gradV;
                    delete tmpMS;
                    delete tmpS;
                    delete prS;
                    return ERR_ANY_OTHER;
                }

                if (resS < resC) {
                    delete _prev;
                    _prev = _curr;
                    _curr = prS;
                    break;
                } else {
                    alpha *= lambda;
                }

                delete tmpMS;
                delete tmpS;
            }

            delete gradV;

            bool res = false;

            if (_curr->eq(_prev, IVector::NORM_INF, res, eps))
                break;
        }
    }
}

Solver1::Solver1():
    _args(NULL), _params(NULL), _prev(NULL), _curr(NULL), _compact(NULL), _problem(NULL)
 {}

Solver1::~Solver1() {
    delete _args;
    delete _params;
    delete _prev;
    delete _curr;
    delete _compact;
}

int Solver1::getId() const {
    return ISolver::INTERFACE_0;
}

int Solver1::setProblem(IProblem * ptr) {
    if (!ptr) {
        ILog::report("ISolver.setProblem: Input parameter ptr is nullptr\n");
        return ERR_WRONG_PROBLEM;
    }
    unsigned dim;
    if (_args) {
        if (ptr->getArgsDim(dim) != ERR_OK) {
            ILog::report("ISolver.setProblem: Cannot check dimensions of arguments\n");
            return ERR_ANY_OTHER;
        }
        if (_args->getDim() != dim) {
            ILog::report("ISolver.setProblem: Dimensions of arguments mismatch\n");
            return ERR_WRONG_PROBLEM;
        }
    }
    if (_params) {
        if (ptr->getParamsDim(dim) != ERR_OK) {
            ILog::report("ISolver.setProblem: Cannot check dimensions of parameters\n");
            return ERR_ANY_OTHER;
        }
        if (_params->getDim() != dim) {
            ILog::report("ISolver.setProblem: Dimensions of parameters mismatch\n");
            return ERR_WRONG_PROBLEM;
        }
    }
    _problem = ptr;
    return ERR_OK;
}

int Solver1::setParams(IVector const* params) {
    if (!params) {
        ILog::report("ISolver.setParams: Input parameter ptr is nullptr\n");
        return ERR_WRONG_ARG;
    }
    unsigned int dim, dimArgs, dimParams, tmp;
    double * coords;
    bool solveByArg;
    double epsilon;
    IVector * args, * param, * begin, * end;
    ICompact * compact;
    if (params->getCoordsPtr(dim, qobject_cast<double const*&>(coords)) != ERR_OK) {
        ILog::report("ISolver.setParams: Cannot get coords from params\n");
        return ERR_ANY_OTHER;
    }
    if (dim < 4) {
        ILog::report("ISolver.setParams: Dimension of params less than 4\n");
        return ERR_WRONG_ARG;
    }
    if (_problem) {
        if (_problem->getArgsDim(dimArgs) != ERR_OK) {
            ILog::report("ISolver.setParams: Cannot check dimensions of arguments\n");
            return ERR_ANY_OTHER;
        }
        if (_problem->getParamsDim(dimParams) != ERR_OK) {
            ILog::report("ISolver.setParams: Cannot check dimensions of parameters\n");
            return ERR_ANY_OTHER;
        }
        if (round(coords[0]) != dimArgs) {
            ILog::report("ISolver.setParams: Dimensions of arguments mismatch\n");
            return ERR_WRONG_ARG;
        }
        if (round(coords[1]) != dimParams) {
            ILog::report("ISolver.setParams: Dimensions of parameters mismatch\n");
            return ERR_WRONG_ARG;
        }
    } else {
        dimArgs = round(coords[0]);
        dimParams = round(coords[1]);
    }
    epsilon = coords[2];
    if (epsilon <= 0) {
        ILog::report("ISolver.setParams: Epsilon is negative\n");
        return ERR_WRONG_ARG;
    }
    tmp = round(coords[3]);
    if (tmp != 1 || tmp != 0) {
        ILog::report("ISolver.setParams: Wrong flag for solve by arguments\n");
        return ERR_WRONG_ARG;
    }
    solveByArg = qobject_cast<bool>(tmp);
    tmp = dimArgs + dimParams + 4;
    if ((solveByArg && dim != tmp + 2 * dimArgs) || (!solveByArg && dim != tmp + 2 * dimParams)) {
        ILog::report("ISolver.setParams: Dimension of params is wrong\n");
        return ERR_WRONG_PROBLEM;
    }
    dim = solveByArg ? dimArgs : dimParams;
    args = IVector::createVector(dimArgs, qobject_cast<double const*&>(coords + 4));
    if (!args) {
        ILog::report("ISolver.setParams: Canntot alloc memory for arguments\n");
        return ERR_MEMORY_ALLOCATION;
    }
    param = IVector::createVector(dimParams, qobject_cast<double const*&>(coords + 4 + dimArgs));
    if (!param) {
        delete args;
        ILog::report("ISolver.setParams: Canntot alloc memory for parameters\n");
        return ERR_MEMORY_ALLOCATION;
    }
    begin = IVector::createVector(dim, qobject_cast<double const*&>(coords + tmp));
    if (!begin) {
        delete args;
        delete params;
        ILog::report("ISolver.setParams: Canntot alloc memory for begin of compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    end = IVector::createVector(dim, qobject_cast<double const*&>(coords + tmp + dim));
    if (!end) {
        delete args;
        delete params;
        delete begin;
        ILog::report("ISolver.setParams: Canntot alloc memory for end of compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    compact = ICompact::createCompact(begin, end);
    if (!compact) {
        delete args;
        delete params;
        delete begin;
        delete end;
        ILog::report("ISolver.setParams: Canntot alloc memory for compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    delete begin;
    delete end;
    delete _args;
    delete _params;
    delete _compact;
    _args = args;
    _params = param;
    _compact = compact;
    solveByArgs = solveByArg;
    eps = epsilon;
    return ERR_OK;
}

int Solver1::setParams(QString & str) {
    unsigned int dim, dimArgs, dimArgs1, dimParams, dimParams1, tmp2;
    QString tmp;
    double * coords;
    bool solveByArg, ok;
    double epsilon;
    IVector * args, * param, * begin, * end;
    ICompact * compact;
    QStringList preparams = str.split(" "), params, tmpList;
    if (preparams.count() < 4) {
        ILog::report("ISolver.setParams: Dimension of params less than 4\n");
        return ERR_WRONG_ARG;
    }
    foreach (QString s, preparams) {
        tmpList = s.split(":");
        if (tmpList.count() != 2) {
            ILog::report("ISolver.setParams: Wrong count of params\n");
            return ERR_WRONG_ARG;
        }
        params.append(s.split(":").at(1));
    }

    if (_problem) {
        if (_problem->getArgsDim(dimArgs) != ERR_OK) {
            ILog::report("ISolver.setParams: Cannot check dimensions of arguments\n");
            return ERR_ANY_OTHER;
        }
        if (_problem->getParamsDim(dimParams) != ERR_OK) {
            ILog::report("ISolver.setParams: Cannot check dimensions of parameters\n");
            return ERR_ANY_OTHER;
        }
        dimArgs1 = params.at(0).toUInt(&ok);
        if (!ok) {
            ILog::report("ISolver.setParams: Cannot get dimension of arguments\n");
            return ERR_WRONG_ARG;
        }
        if (!ok || dimArgs1 != dimArgs) {
            ILog::report("ISolver.setParams: Dimensions of arguments mismatch\n");
            return ERR_WRONG_ARG;
        }
        dimParams1 = params.at(1).toUInt(&ok);
        if (!ok) {
            ILog::report("ISolver.setParams: Cannot get dimension of parameters\n");
            return ERR_WRONG_ARG;
        }
        if (dimParams1 != dimParams) {
            ILog::report("ISolver.setParams: Dimensions of parameters mismatch\n");
            return ERR_WRONG_ARG;
        }
    } else {
        dimArgs = params.at(0).toUInt(&ok);
        if (!ok) {
            ILog::report("ISolver.setParams: Cannot get dimension of arguments\n");
            return ERR_WRONG_ARG;
        }
        dimParams = params.at(1).toUInt(&ok);
        if (!ok) {
            ILog::report("ISolver.setParams: Cannot get dimension of parameters\n");
            return ERR_WRONG_ARG;
        }
    }
    epsilon = params.at(2).toDouble(&ok);
    if (!ok) {
        ILog::report("ISolver.setParams: Cannot get epsilon\n");
        return ERR_WRONG_ARG;
    }
    if (epsilon <= 0) {
        ILog::report("ISolver.setParams: Epsilon is negative\n");
        return ERR_WRONG_ARG;
    }
    tmp = params.at(3);
    if (tmp.length() == 0) {
        ILog::report("ISolver.setParams: Wrong string for solve by arguments or parameters\n");
        return ERR_WRONG_ARG;
    } else if (QRegExp("[Aa][Rr][Gg][Ss]").exactMatch(tmp)) {
        solveByArg = true;
    } else if (QRegExp("[Pp][Aa][Rr][Aa][Mm][Ss]").exactMatch(tmp)) {
        solveByArg = false;
    } else {
        ILog::report("ISolver.setParams: Wrong string for solve by arguments or parameters\n");
        return ERR_WRONG_ARG;
    }
    if (params.count() != dimArgs + dimParams + 4 + 2 * std::max(dimArgs, dimParams)) {
        ILog::report("ISolver.setParams: Count of params is wrong\n");
        return ERR_WRONG_PROBLEM;
    }
    dim = solveByArg ? dimArgs : dimParams;
    coords = new (std::nothrow) double[std::max(dimArgs, dimParams)];
    if (!coords) {
        ILog::report("ISolver.setParams: Canntot alloc memory\n");
        return ERR_MEMORY_ALLOCATION;
    }
    tmp2 = 4;
    for (int i = 0; i < dimArgs; ++i) {
        coords[i] = params.at(i + tmp2).toDouble(&ok);
        if (!ok) {
            delete coords;
            ILog::report("ISolver.setParams: Cannot get args\n");
            return ERR_ANY_OTHER;
        }
    }
    args = IVector::createVector(dimArgs, qobject_cast<double const*&>(coords));
    if (!args) {
        delete coords;
        ILog::report("ISolver.setParams: Canntot alloc memory for arguments\n");
        return ERR_MEMORY_ALLOCATION;
    }
    tmp2 += dimArgs;
    for (int i = 0; i < dimParams; ++i) {
        coords[i] = params.at(i + tmp2).toDouble(&ok);
        if (!ok) {
            delete coords;
            delete args;
            ILog::report("ISolver.setParams: Cannot get params\n");
            return ERR_ANY_OTHER;
        }
    }
    param = IVector::createVector(dimParams, qobject_cast<double const*&>(coords));
    if (!param) {
        delete coords;
        delete args;
        ILog::report("ISolver.setParams: Canntot alloc memory for parameters\n");
        return ERR_MEMORY_ALLOCATION;
    }
    tmp += dimParams;
    for (int i = 0; i < dim; ++i) {
        coords[i] = params.at(i + tmp2).toDouble(&ok);
        if (!ok) {
            delete coords;
            delete args;
            delete param;
            ILog::report("ISolver.setParams: Cannot get begin of compact\n");
            return ERR_ANY_OTHER;
        }
    }
    begin = IVector::createVector(dim, qobject_cast<double const*&>(coords));
    if (!begin) {
        delete coords;
        delete args;
        delete param;
        ILog::report("ISolver.setParams: Canntot alloc memory for begin of compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    tmp += std::max(dimArgs, dimParams);
    for (int i = 0; i < dim; ++i) {
        coords[i] = params.at(i + tmp2).toDouble(&ok);
        if (!ok) {
            delete coords;
            delete args;
            delete param;
            delete begin;
            ILog::report("ISolver.setParams: Cannot get end of compact\n");
            return ERR_ANY_OTHER;
        }
    }
    end = IVector::createVector(dim, qobject_cast<double const*&>(coords));
    if (!end) {
        delete coords;
        delete args;
        delete param;
        delete begin;
        ILog::report("ISolver.setParams: Canntot alloc memory for end of compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    delete coords;
    compact = ICompact::createCompact(begin, end);
    if (!compact) {
        delete args;
        delete param;
        delete begin;
        delete end;
        ILog::report("ISolver.setParams: Canntot alloc memory for compact\n");
        return ERR_MEMORY_ALLOCATION;
    }
    delete begin;
    delete end;
    delete _args;
    delete _params;
    delete _compact;
    _args = args;
    _params = param;
    _compact = compact;
    solveByArgs = solveByArg;
    eps = epsilon;
    return ERR_OK;
}

int Brocker2::getId() const {
    return IBrocker::INTERFACE_0;
}

int Solver1::getQml(QUrl& qml) const {
    QString file = "qrc:/solver1.qml";
    if (!QFile::exists(file))
        return ERR_ANY_OTHER;
    qml = QUrl::fromLocalFile(file);
    return ERR_OK;
}

bool Brocker2::canCastTo(Type type) const {
    switch (type) {
    case IBrocker::SOLVER:
        return true;
    default:
        return false;
    }
}

void* Brocker2::getInterfaceImpl(Type type) const {
    switch (type) {
    case IBrocker::SOLVER:
        return _solver;
    default:
        return NULL;
    }
}

int Brocker2::release() {
    delete this;

    return ERR_OK;
}

Brocker2::Brocker2(Solver1 *solver):
    _solver(solver)
{}

Brocker2::~Brocker2() {
    delete _solver;
}

extern "C" {
SHARED_EXPORT void* getBrocker() {
    Solver1 *solver = new (std::nothrow) Solver1();

    if (!solver) {
        ILog::report("getBrocker: not enough memory\n");
        return NULL;
    }

    Brocker2 *brocker = new (std::nothrow) Brocker2(solver);

    if (!brocker) {
        ILog::report("getBrocker: not enough memory\n");
        delete solver;
        return NULL;
    }

    return brocker;
}
}
