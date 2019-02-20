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