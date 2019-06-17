#include <QVector>
#include <cmath>
#include "ISet.h"
#include "error.h"
#include "ILog.h"

const double EPS = 1e-8;

namespace {
    class ISetImpl : public ISet
    {
    public:
        int getId() const;

        int put(IVector const* const item);
        int get(unsigned int index, IVector*& pItem) const;
        int remove(unsigned int index);
        int contains(IVector const* const pItem, bool& rc) const;
        unsigned int getSize() const;
        int clear();

        IIterator* end();
        IIterator* begin();

        int deleteIterator(IIterator * pIter);
        int getByIterator(IIterator const* pIter, IVector*& pItem) const;

        class IIteratorImpl : public ISet::IIterator
        {
        public:
            int next();
            int prev();
            bool isEnd() const;
            bool isBegin() const;

            ISet const* const _set;
            unsigned int _pos;

            IIteratorImpl(ISet const* const set, int pos);
        };

        /*ctor*/
        ISetImpl(uint dim);
        /*dtor*/
        ~ISetImpl();


    private:
        QVector<IVector*> _ptr_points;
        QVector<IIteratorImpl*> _ptr_iterators;
        unsigned int _dim;
    };

} //end anonymous namespace

int ISetImpl::getId() const {
    return ISet::INTERFACE_0;
}

ISet* ISet::createSet(unsigned int R_dim) {
    if (R_dim == 0) {
        ILog::report("ISet.createSet: Can't create Set with zero dimension\n");
        return NULL;
    }

    ISet* set = new(std::nothrow) ISetImpl(R_dim);
    if (!set) {
        ILog::report("ISet.createSet: Not enough memory\n");
        return NULL;
    }
    return set;
}

ISetImpl::ISetImpl(uint dim) {
    _dim = dim;
}

ISetImpl::~ISetImpl() {
    for (int i = 0; i < _ptr_points.size(); i++) {
        delete _ptr_points[i];
    }
    for (int i = 0; i < _ptr_iterators.size(); i++) {
        delete _ptr_iterators[i];
    }
}

int ISetImpl::put(IVector const* const item) {
    if (!item) {
        ILog::report("ISet.put: Input argument is nullptr\n");
        return ERR_WRONG_ARG;
    }
    if  (_dim != item->getDim()) {
        ILog::report("ISet.put: Input argument has another dimension\n");
        return ERR_DIMENSIONS_MISMATCH;
    }

    IVector* temp = item->clone();
    if (!temp) {
        ILog::report("ISet.put: Not enough memory\n");
        return ERR_MEMORY_ALLOCATION;
    }

    _ptr_points.append(temp);
    return ERR_OK;
}

int ISetImpl::get(unsigned int index, IVector*& pItem) const {
    if  (index >= (uint)_ptr_points.size()) {
        ILog::report("ISet.get: Wrong index (out of range)\n");
        return ERR_OUT_OF_RANGE;
    }

    pItem = _ptr_points[index]->clone();
    if (!pItem) {
        ILog::report("ISet.get: Not enough memory\n");
        return ERR_MEMORY_ALLOCATION;
    }

    return ERR_OK;
}

int ISetImpl::remove(unsigned int index) {
    if  (index >= (uint)_ptr_points.size()) {
        ILog::report("ISet.remove: Wrong index (out of range)\n");
        return ERR_OUT_OF_RANGE;
    }

    for (int i = 0; i < _ptr_iterators.size(); i++) {
        if (_ptr_iterators[i]->_pos > index) {
            _ptr_iterators[i]->_pos--;
        }
        else if (_ptr_iterators[i]->_pos == index) {
            deleteIterator(_ptr_iterators[i]);
        }
    }

    _ptr_points.remove(index);

    return ERR_OK;
}

int ISetImpl::contains(IVector const* const pItem, bool& rc) const {
    if (!pItem) {
        ILog::report("ISet.contains: Input argument is nullptr\n");
        return ERR_WRONG_ARG;
    }
    if  (_dim != pItem->getDim()) {
        ILog::report("ISet.contains: Input argument has another dimension\n");
        return ERR_DIMENSIONS_MISMATCH;
    }

    int errCode;
    rc = false;
    for (int i = 0; i < _ptr_points.size(); i++) {
        errCode = pItem->eq(_ptr_points[i], IVector::NORM_INF, rc, EPS);
        if (errCode != ERR_OK)
            return errCode;
        if (rc)
            break;
    }
    return ERR_OK;
}

unsigned int ISetImpl::getSize() const {
    return _ptr_points.size();
}

int ISetImpl::clear() {
    for (int i = 0; i < _ptr_points.size(); i++) {
        delete _ptr_points[i];
    }
    _ptr_points.clear();

    for (int i = 0; i < _ptr_iterators.size(); i++) {
        delete _ptr_iterators[i];
    }
    _ptr_iterators.clear();

    return ERR_OK;
}

ISetImpl::IIterator* ISetImpl::end() {
    if (_ptr_points.size() == 0) {
        ILog::report("ISet.end: Can not create iterator of empty set\n");
        return NULL;
    }
    ISetImpl::IIteratorImpl* iterator
            = new(std::nothrow) ISetImpl::IIteratorImpl::IIteratorImpl(this, _ptr_points.size() - 1);
    if (!iterator) {
        ILog::report("ISet.end: Not enough memory\n");
        return NULL;
    }
     _ptr_iterators.append(iterator);
    return iterator;
}

ISetImpl::IIterator* ISetImpl::begin() {
    if (_ptr_points.size() == 0) {
        ILog::report("ISet.begin: Can not create iterator of empty set\n");
        return NULL;
    }
    ISetImpl::IIteratorImpl* iterator
            = new(std::nothrow) ISetImpl::IIteratorImpl::IIteratorImpl(this, 0);
    if (!iterator) {
        ILog::report("ISet.begin: Not enough memory\n");
        return NULL;
    }
    _ptr_iterators.append(iterator);
    return iterator;
}

int findIterator(QVector<ISetImpl::IIteratorImpl*> ptr_iterators, ISet::IIterator * pIter) {
    for (int i = 0; i < ptr_iterators.size(); i++) {
        if (dynamic_cast<ISet::IIterator>(ptr_iterators[i]) == pIter) {
            return i;
        }
    }
    return -1;
}

int ISetImpl::deleteIterator(IIterator * pIter) {
    if (!pIter) {
        ILog::report("ISet.deleteIterator: Input argument is nullptr\n");
        return ERR_WRONG_ARG;
    }

    int indIterator = findIterator(_ptr_iterators, pIter);

    if (indIterator == -1) {
        ILog::report("ISet.deleteIterator: Set does not contain input iterator\n");
        return ERR_WRONG_ARG;
    }
    else {
        delete _ptr_iterators[indIterator];
        _ptr_iterators.remove(indIterator);
        return ERR_OK;
    }
}

int ISetImpl::getByIterator(IIterator const* pIter, IVector*& pItem) const {
    if (!pIter) {
        ILog::report("ISet.getByIterator: Input argument is nullptr\n");
        return ERR_WRONG_ARG;
    }

    int indIterator = findIterator(_ptr_iterators, pIter);

    if (indIterator == -1) {
        ILog::report("ISet.getByIterator: Set does not contain input iterator\n");
        return ERR_WRONG_ARG;
    }
    else {
        return _ptr_iterators[i]->_set->get(_ptr_iterators[i]->_pos, pItem);
    }
}

int ISetImpl::IIteratorImpl::next() {
    if (_pos + 1 >= _set->getSize()) {
        ILog::report("ISet.next: Iterator already at the end of the set\n");
        return ERR_OUT_OF_RANGE;
    }
    _pos++;
    return ERR_OK;
}

int ISetImpl::IIteratorImpl::prev() {
    if (_pos == 0) {
        ILog::report("ISet.next: Iterator already at the begin of the set\n");
        return ERR_OUT_OF_RANGE;
    }
    _pos--;
    return ERR_OK;
}

bool ISetImpl::IIteratorImpl::isEnd() const {
    return _pos == _set->getSize() - 1;
}

bool ISetImpl::IIteratorImpl::isBegin() const {
    return _pos == 0;
}

ISet::IIterator::IIterator(const ISet *const set, int pos) {}

ISetImpl::IIteratorImpl::IIteratorImpl(ISet const* const set, int pos): ISet::IIterator(set, pos), _set(set), _pos(pos) {}
