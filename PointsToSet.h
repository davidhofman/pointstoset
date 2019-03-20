class BitvectorPointsToSet {
    ADT::SparseBitvector nodes;
    ADT::SparseBitvector offsets;
    static std::map<PSNode*,size_t> ids;

    size_t getNodeID(PSNode *node) const {
        auto it = ids.find(node);
        if(it != ids.end())
            return it->second;
        return ids.emplace_hint(it, node, ids.size() + 1)->second;
    }
    
public:
    bool add(PSNode *target, Offset off) {
        bool changed = nodes.set(getNodeID(target));
        return offsets.set(*off) || changed;
    }

    bool add(const Pointer& ptr) {
        return add(ptr.target, ptr.offset);
    }

    bool add(const BitvectorPointsToSet& S) {
        bool changed = nodes.set(S.nodes);
        return offsets.set(S.offsets) || changed;
    }

    bool remove(const Pointer& ptr) {
        abort();
    }
    
    bool remove(PSNode *target, Offset offset) {
        abort();
    }
    
    bool removeAny(PSNode *target) {
        abort();
        /*
        bool changed = nodes.unset(getNodeID(target));
        if(nodes.empty()) {
            offsets.reset();
        }
        return changed;
        */
    }
    
    void clear() { 
        nodes.reset();
        offsets.reset();
    }
   
    bool pointsTo(const Pointer& ptr) const {
        return nodes.get(getNodeID(ptr.target)) && offsets.get(*ptr.offset);
    }

    bool mayPointTo(const Pointer& ptr) const {
        return pointsTo(ptr);
    }

    bool mustPointTo(const Pointer& ptr) const {
        return (nodes.size() == 1 || offsets.size() == 1)
                && pointsTo(ptr);
    }

    bool pointsToTarget(PSNode *target) const {
        return nodes.get(getNodeID(target));
    }

    bool isSingleton() const {
        return nodes.size() == 1 && offsets.size() == 1;
    }

    bool empty() const {
        return nodes.empty() && offsets.empty();
    }

    size_t count(const Pointer& ptr) const {
        return pointsTo(ptr);
    }

    bool has(const Pointer& ptr) const {
        return count(ptr) > 0;
    }

    size_t size() const {
        return nodes.size() * offsets.size();
    }

    void swap(BitvectorPointsToSet& rhs) {
        nodes.swap(rhs.nodes);
        offsets.swap(rhs.offsets);
    }
};

class BitvectorPointsToSet2 {
    ADT::SparseBitvector pointers;
    static std::map<Pointer, size_t> ids;

    size_t getPointerID(Pointer ptr) const {
        auto it = ids.find(ptr);
        if(it != ids.end())
            return it->second;
        return ids.emplace_hint(it, ptr, ids.size() + 1)->second;
    }
    
public:
    bool add(PSNode *target, Offset off) {
        return add(Pointer(target,off));
    }

    bool add(const Pointer& ptr) {
        return pointers.set(getPointerID(ptr));
    }

    bool add(const BitvectorPointsToSet2& S) {
        return pointers.set(S.pointers);
    }

    bool remove(const Pointer& ptr) {
        return pointers.unset(getPointerID(ptr));
    }
    
    bool remove(PSNode *target, Offset offset) {
        return remove(Pointer(target,offset));
    }
    
    bool removeAny(PSNode *target) {
        bool changed = false;
        for(const auto& kv : ids) {
            if(kv.first.target == target) {
                changed |= pointers.unset(kv.second);
            }
        }
        return changed;
    }
    
    void clear() { 
        pointers.reset();
    }
   
    bool pointsTo(const Pointer& ptr) const {
        return pointers.get(getPointerID(ptr));
    }

    bool mayPointTo(const Pointer& ptr) const {
        return pointsTo(ptr) || pointsTo(Pointer(ptr.target, Offset::UNKNOWN));
    }

    bool mustPointTo(const Pointer& ptr) const {
        assert(!ptr.offset.isUnknown() && "Makes no sense");
        return pointsTo(ptr) && isSingleton();
    }

    bool pointsToTarget(PSNode *target) const {
        for(const auto& kv : ids) {
            if(kv.first.target == target && pointers.get(kv.second)) {
                return true;
            }
        }
        return false;
    }

    bool isSingleton() const {
        return pointers.size() == 1;
    }

    bool empty() const {
        return pointers.empty();
    }

    size_t count(const Pointer& ptr) const {
        return pointsTo(ptr);
    }

    bool has(const Pointer& ptr) const {
        return count(ptr) > 0;
    }

    size_t size() const {
        return pointers.size();
    }

    void swap(BitvectorPointsToSet2& rhs) {
        pointers.swap(rhs.pointers);
    }
};

class BitvectorPointsToSet3 {
    
    ADT::SparseBitvector pointers;
    std::set<Pointer> largePointers;
    static std::map<PSNode*,size_t> ids;

    size_t getNodeID(PSNode *node) const {
        auto it = ids.find(node);
        if(it != ids.end())
            return it->second;
        return ids.emplace_hint(it, node, ids.size() + 1)->second;
    }
    
    size_t getNodePosition(PSNode *node) const {
        return ((getNodeID(node) - 1) * 64);
    }
    
public:
    bool add(PSNode *target, Offset off) {
        if(off.isUnknown()) {
            return pointers.set(getNodePosition(target) + 63); 
        } else if(off < 63) {
            return pointers.set(getNodePosition(target) + off.offset);
        } else {
            return largePointers.emplace(target, off).second;
        }
    }

    bool add(const Pointer& ptr) {
        return add(ptr.target, ptr.offset);
    }

    bool add(const BitvectorPointsToSet3& S) {
        bool changed = pointers.set(S.pointers);
        for (const auto& ptr : S.largePointers) {
            changed |= largePointers.insert(ptr).second;
        }
        return changed;
    }

    bool remove(const Pointer& ptr) {
        if(ptr.offset.isUnknown()) {
            return pointers.unset(getNodePosition(ptr.target) + 63); 
        } else if(ptr.offset < 63) {
            return pointers.unset(getNodePosition(ptr.target) + ptr.offset.offset);
        } else {
            return largePointers.erase(ptr) != 0;
        }
    }
    
    bool remove(PSNode *target, Offset offset) {
        return remove(Pointer(target,offset));
    }
    
    bool removeAny(PSNode *target) {
        bool changed = false;
        size_t position = getNodePosition(target);
        for(size_t i = position; i <  position + 64; i++) {
            changed |= pointers.unset(i);
        }
        auto it = largePointers.begin();
        while(it != largePointers.end()) {
            if(it->target == target) {
                it = largePointers.erase(it);
                changed = true;
            }
            else {
                it++;
            }
        }
        return changed;
    }
    
    void clear() { 
        pointers.reset();
        largePointers.clear();
    }
   
    bool pointsTo(const Pointer& ptr) const {
        return pointers.get(getNodePosition(ptr.target) + ptr.offset.offset)
                || largePointers.find(ptr) != largePointers.end();
    }

    bool mayPointTo(const Pointer& ptr) const {
        return pointsTo(ptr)
                || pointsTo(Pointer(ptr.target, Offset::UNKNOWN));
    }

    bool mustPointTo(const Pointer& ptr) const {
        assert(!ptr.offset.isUnknown() && "Makes no sense");
        return pointsTo(ptr) && isSingleton();
    }

    bool pointsToTarget(PSNode *target) const {
        size_t position = getNodePosition(target);
        for(size_t i = position; i <  position + 64; i++) {
            if(pointers.get(i))
                return true;
        }
        for (const auto& ptr : largePointers) {
            if (ptr.target == target)
                return true;
        }
        return false;
    }

    bool isSingleton() const {
        return (pointers.size() == 1 && largePointers.size() == 0)
                || (pointers.size() == 0 && largePointers.size() == 1);
    }

    bool empty() const {
        return pointers.size() == 0
                && largePointers.size() == 0;
    }

    size_t count(const Pointer& ptr) const {
        return pointsTo(ptr);
    }

    bool has(const Pointer& ptr) const {
        return count(ptr) > 0;
    }

    size_t size() const {
        return pointers.size() + largePointers.size();
    }

    void swap(BitvectorPointsToSet3& rhs) {
        pointers.swap(rhs.pointers);
        largePointers.swap(rhs.largePointers);
    }
};

class BitvectorPointsToSet4 {
    
    const unsigned int multiplier = 4;
    
    ADT::SparseBitvector pointers;
    std::set<Pointer> oddPointers;
    static std::map<PSNode*,size_t> ids;

    size_t getNodeID(PSNode *node) const {
        auto it = ids.find(node);
        if(it != ids.end())
            return it->second;
        return ids.emplace_hint(it, node, ids.size() + 1)->second;
    }
    
    size_t getNodePosition(PSNode *node) const {
        return ((getNodeID(node) - 1) * 64);
    }
    
    size_t getOffsetPosition(PSNode *node, Offset off) const {
        if(off.isUnknown()) {
            return getNodePosition(node) + 63;
        }
        return getNodePosition(node) + (*off / multiplier);
    }
    
    bool isOffsetValid(Offset off) const {
        return off.isUnknown() 
                || (*off <= 62 * multiplier && *off % multiplier == 0);
    }
    
public:
    bool add(PSNode *target, Offset off) {
        if(isOffsetValid(off)) {
            return pointers.set(getOffsetPosition(target, off));
        }
        return oddPointers.emplace(target,off).second;
        
    }

    bool add(const Pointer& ptr) {
        return add(ptr.target, ptr.offset);
    }

    bool add(const BitvectorPointsToSet4& S) {
        bool changed = pointers.set(S.pointers);
        for (const auto& ptr : S.oddPointers) {
            changed |= oddPointers.insert(ptr).second;
        }
        return changed;
    }

    bool remove(const Pointer& ptr) {
        if(isOffsetValid(ptr.offset)) {
            return pointers.unset(getOffsetPosition(ptr.target, ptr.offset)); 
        }
        return oddPointers.erase(ptr) != 0;
    }
    
    bool remove(PSNode *target, Offset offset) {
        return remove(Pointer(target,offset));
    }
    
    bool removeAny(PSNode *target) {
        bool changed = false;
        size_t position = getNodePosition(target);
        for(size_t i = position; i <  position + 64; i++) {
            changed |= pointers.unset(i);
        }
        auto it = oddPointers.begin();
        while(it != oddPointers.end()) {
            if(it->target == target) {
                it = oddPointers.erase(it);
                changed = true;
            }
            else {
                it++;
            }
        }
        return changed;
    }
    
    void clear() { 
        pointers.reset();
        oddPointers.clear();
    }
   
    bool pointsTo(const Pointer& ptr) const {
        return pointers.get(getOffsetPosition(ptr.target,ptr.offset))
                || oddPointers.find(ptr) != oddPointers.end();
    }

    bool mayPointTo(const Pointer& ptr) const {
        return pointsTo(ptr)
                || pointsTo(Pointer(ptr.target, Offset::UNKNOWN));
    }

    bool mustPointTo(const Pointer& ptr) const {
        assert(!ptr.offset.isUnknown() && "Makes no sense");
        return pointsTo(ptr) && isSingleton();
    }

    bool pointsToTarget(PSNode *target) const {
        size_t position = getNodePosition(target);
        for(size_t i = position; i <  position + 64; i++) {
            if(pointers.get(i))
                return true;
        }
        for (const auto& ptr : oddPointers) {
            if (ptr.target == target)
                return true;
        }
        return false;
    }

    bool isSingleton() const {
        return (pointers.size() == 1 && oddPointers.size() == 0)
                || (pointers.size() == 0 && oddPointers.size() == 1);
    }

    bool empty() const {
        return pointers.size() == 0
                && oddPointers.size() == 0;
    }

    size_t count(const Pointer& ptr) const {
        return pointsTo(ptr);
    }

    bool has(const Pointer& ptr) const {
        return count(ptr) > 0;
    }

    size_t size() const {
        return pointers.size() + oddPointers.size();
    }

    void swap(BitvectorPointsToSet4& rhs) {
        pointers.swap(rhs.pointers);
        oddPointers.swap(rhs.oddPointers);
    }
};