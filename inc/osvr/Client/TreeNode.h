/** @file
    @brief Header

    @date 2014

    @author
    Ryan Pavlik
    <ryan@sensics.com>
    <http://sensics.com>
*/

// Copyright 2014 Sensics, Inc.
//
// All rights reserved.
//
// (Final version intended to be licensed under
// the Apache License, Version 2.0)

#ifndef INCLUDED_TreeNode_h_GUID_EEC6C5AF_332A_4780_55C2_D794B3BF5106
#define INCLUDED_TreeNode_h_GUID_EEC6C5AF_332A_4780_55C2_D794B3BF5106

// Internal Includes
#include <osvr/Client/TreeNode_fwd.h>

// Library/third-party includes
#include <boost/noncopyable.hpp>
#include <boost/assert.hpp>

// Standard includes
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>

namespace osvr {
namespace client {
    template <typename ValueType>
    class TreeNode : public enable_shared_from_this<TreeNode<ValueType> >,
                     boost::noncopyable {
      public:
        /// @brief This template instantiation's type
        typedef TreeNode<ValueType> type;

        /// @brief The pointer for holding this template instantiation
        typedef typename TreeNodePointer<type>::type ptr_type;

        /// @brief The weak pointer for accessing this template instantiation
        typedef weak_ptr<type> weak_ptr_type;

        /// @brief The contained value type
        typedef ValueType value_type;

        /// @brief Create a path node
        static type &create(TreeNode &parent, std::string const &name);

        /// @brief Create a root.
        static ptr_type createRoot();

        /// @brief Get the named child, creating it if it doesn't exist.
        type &getOrCreateChildByName(std::string const &name);

        std::string const &getName() const;

        bool isRoot() const;

        value_type &get() { return m_value; }
        value_type const &get() const { return m_value; }

        template <typename F> void visitChildren(F &visitor) {
            std::for_each(begin(m_children), end(m_children), visitor);
        }

        template <typename F> void visitConstChildren(F &visitor) const {
            std::for_each(begin(m_children), end(m_children), visitor);
        }

      private:
        /// @brief Private constructor for a non-root node
        TreeNode(type &parent, std::string const &name);

        /// @brief Private constructor for a root node
        TreeNode();

        /// @brief Internal helper to get child by name, or a null pointer if no
        /// such child.
        ptr_type m_getChildByName(std::string const &name);

        /// @brief Internal helper to add a named child. Assumes no such child
        /// already exists!
        void m_addChild(ptr_type const &child);

        /// @brief Contained value.
        value_type m_value;

        typedef std::vector<ptr_type> ChildList;
        /// @brief Ownership of children
        ChildList m_children;

        /// @brief Name
        std::string const m_name;

        /// @brief Weak pointer to parent.
        weak_ptr_type m_parent;
    };

    template <typename ValueType>
    inline TreeNode<ValueType> &
    TreeNode<ValueType>::create(TreeNode<ValueType> &parent,
                                std::string const &name) {
        if (parent.m_getChildByName(name)) {
            throw std::logic_error("Can't create a child with the same name as "
                                   "an existing child!");
        }
        ptr_type ret(new TreeNode(parent, name));
        parent.m_addChild(ret);
        return *ret;
    }

    template <typename ValueType>
    inline typename TreeNode<ValueType>::ptr_type
    TreeNode<ValueType>::createRoot() {
        ptr_type ret(new TreeNode());
        return ret;
    }

    template <typename ValueType>
    inline TreeNode<ValueType> &
    TreeNode<ValueType>::getOrCreateChildByName(std::string const &name) {
        ptr_type child = m_getChildByName(name);
        if (child) {
            return *child;
        }
        return TreeNode::create(*this, name);
    }

    template <typename ValueType>
    inline std::string const &TreeNode<ValueType>::getName() const {
        return m_name;
    }

    template <typename ValueType>
    inline typename TreeNode<ValueType>::ptr_type
    TreeNode<ValueType>::m_getChildByName(std::string const &name) {
        /// @todo Don't use a linear search here - use an unordered map or
        /// something.
        typename ChildList::const_iterator it = std::find_if(
            begin(m_children), end(m_children),
            [&](ptr_type const &n) { return n->getName() == name; });
        ptr_type ret;
        if (it != end(m_children)) {
            ret = *it;
        }
        return ret;
    }

    template <typename ValueType>
    inline void TreeNode<ValueType>::m_addChild(
        typename TreeNode<ValueType>::ptr_type const &child) {
        m_children.push_back(child);
    }

    template <typename ValueType>
    inline TreeNode<ValueType>::TreeNode(TreeNode<ValueType> &parent,
                                         std::string const &name)
        : m_name(name), m_parent(parent.shared_from_this()), m_children() {
        if (m_name.empty()) {
            throw std::logic_error(
                "Can't create a named path node with an empty name!");
        }
    }

    template <typename ValueType>
    inline TreeNode<ValueType>::TreeNode()
        : m_name(), m_parent(), m_children() {
        /// Special root constructor
    }
    template <typename ValueType>
    inline bool TreeNode<ValueType>::isRoot() const {
        BOOST_ASSERT_MSG(m_parent.expired() == m_name.empty(),
                         "The root and only the root should have an empty name "
                         "and no parent!");
        return m_name.empty();
    }

} // namespace client
} // namespace osvr

#endif // INCLUDED_TreeNode_h_GUID_EEC6C5AF_332A_4780_55C2_D794B3BF5106
