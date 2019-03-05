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
        if(nodes.empty())
            offsets.reset();
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