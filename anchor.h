// anchor.h


#include <iostream>

#ifndef ANCHOR_H_INCLUDED
#define ANCHOR_H_INCLUDED


template <typename T>
class Anchor
{
public:
	Anchor():children(0)
	{
		link.first=0;
		link.second = 0;
		bridge = 0;
		//std::cout << "crea" << *this << std::endl;
	}
	Anchor(unsigned int id)
	{
		link.first = id;
		link.second = this;
		bridge = 0;
		children = 1;
		//std::cout << "crea_ui " << *this << std::endl;
	}
	Anchor(unsigned int id, Anchor* p)
	{
		link.first = id;
		link.second = p;
		bridge = 0;
		//std::cout << "crea_ui+cy " << *this << std::endl;
	}
	Anchor(const Anchor& Anchor)
	{
		link.first = Anchor.link.first;
		link.second = this;
		bridge = 0;
		children = 1;		
		//std::cout << "crea_cy " << *this << std::endl;
	}
	~Anchor()
	{
		//std::cout << "dest " << *this << std::endl;
	}

	bool operator<(const Anchor& a) const
	{
		return link.first < a.link.first;
	}
	bool operator>(const Anchor& a) const
	{
		return link.first > a.link.first;
	}
	bool operator==(const Anchor& a) const
	{
		return link.first == a.link.first;
	}
        bool operator!=(const Anchor& a) const
        {
                return link.first != a.link.first;
        }
	inline unsigned int Id()
	{
		return link.first;
	}
        inline const unsigned int& Id() const
        {
                return link.first;
        }
	inline Anchor* Next()
	{
		return link.second;
	}
	void SetNext(Anchor * anc)
	{
		link.second = anc;
	}
        inline const Anchor* Next() const
        {
                return link.second;
        }
	inline Anchor* Intersection()
	{
		return bridge;
	}
	inline const Anchor* Intersection() const
	{
		return bridge;
	}
	void SetIntersection(Anchor * anc)
	{
		bridge = anc;
	}
	inline unsigned int Children() const
	{
		return children;
	}
	inline void DecChildren()
	{
		--children;
	}
	inline void IncChildren()
	{
		++children;
	}
	inline void SetChildren(unsigned int c)
	{
		children = c;
	}
	void Reset()
	{
		link.second = this;
		bridge = 0;
	}
	// Obsolète. 
	void ResetObsolete()
	{
		--(link.second -> children);
		//++children;
		link.second = this;
		bridge = 0;
	}
	Anchor * Clone()
	{
		return new Anchor(*this);
	}
	Anchor& operator*()
	{
        	return *this;
    	}
private:
	std::pair<T, Anchor*> 	link;
	bool 			is_root;
	Anchor* 		bridge;
	unsigned int		children;
	friend std::ostream& operator<< (std::ostream& os, const Anchor& Anchor)
	{
		os << "(" << Anchor.Id() << "," << Anchor.Next() -> Id() << ")";
		return os;
	}
	//Partition*		partition;
};


typedef Anchor<unsigned int> * PAnchor;


class Membership
{
public:
	Membership() { panchor = 0; };
	~Membership() { }
	Membership(unsigned int& e, Anchor<unsigned int>* b) { id = e; panchor = b; }
	Membership(unsigned int& e, unsigned int& b) { id = e; panchor = new Anchor<unsigned int>(b); }
	unsigned int GetId() { return id; }
	Anchor<unsigned int> * GetPAnchor() { return panchor; }
	void SetId(unsigned int e) { id = e; }
	void SetPAnchor(Anchor<unsigned int> * pa) { panchor = pa; }
private:
	unsigned int id;
	Anchor<unsigned int> * panchor;
};



#endif
