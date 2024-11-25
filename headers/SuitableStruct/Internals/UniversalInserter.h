/* License:  MIT
 * Source:   https://github.com/ihor-drachuk/SuitableStruct
 * Contact:  ihor-drachuk-libs@pm.me  */

#pragma once
#include <iterator>

namespace SuitableStruct {

template<typename T>
struct UniversalInserterContext {};

template<typename Container>
struct UniversalInserterEmptySetup
{
    void operator()(UniversalInserterContext<Container>&, Container&) const { }
};

template<typename Container, typename Inserter, typename Setup = UniversalInserterEmptySetup<Container>>
class UniversalInserter {
public:
    using This = UniversalInserter<Container, Inserter, Setup>;
    using iterator_category = std::forward_iterator_tag;
    using value_type        = typename Container::value_type;
    using difference_type   = typename Container::difference_type;
    using pointer           = typename Container::const_pointer;
    using reference         = const value_type&;

    UniversalInserter(Container& container, const Inserter& inserter, const Setup& setup = Setup())
        : m_container(container),
          m_inserter(inserter)
    {
        setup(m_context, m_container);
    }

    This& operator=(const value_type& value)
    {
        m_inserter(m_context, m_container, value);
        return (*this);
    }

    This& operator=(value_type&& value)
    {
        m_inserter(m_context, m_container, std::move(value));
        return (*this);
    }

    This& operator*()
    {
        return (*this);
    }

    This& operator++(int) // Post-increment
    {
        return (*this);
    }

private:
    UniversalInserterContext<Container> m_context;
    Container& m_container;
    Inserter m_inserter;
};

} // namespace SuitableStruct
