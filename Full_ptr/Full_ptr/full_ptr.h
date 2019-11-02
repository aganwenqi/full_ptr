


class father_ptr
{
public:
	template<typename T>
	class _ptr
	{
	public:
		_ptr(T & t) :_data(&t){ count = 0; }
		
		T & GetData()
		{
			return *_data;
		}
		void operator++(int){ count++; }
		void operator--(int)
		{
			if (--count <= 0)
			{
				delete _data;
				delete(this);
			}
			
		}
	private:
		int count;
		T * _data;
	};
};
template<typename T>
class full_ptr : public father_ptr
{
public:
	full_ptr(T * t)
	{
		m_ptr = new _ptr<T>(*t);
		(*m_ptr)++;
	}
	full_ptr(T && t)
	{
		m_ptr = new _ptr<T>(*(new T(t)));
		(*m_ptr)++;
	}
	full_ptr(T & t)
	{
		m_ptr = new _ptr<T>(*(new T(t)));
		(*m_ptr)++;
	}
	full_ptr(const full_ptr & ptr)
	{
		if (m_ptr != nullptr)
		{
			(*m_ptr)--;
		}
		m_ptr = ptr.m_ptr;
		(*m_ptr)++;
	}

	T * operator->()
	{
		return &m_ptr->GetData();
	}
	void operator=(full_ptr & ptr)
	{
		if (m_ptr != nullptr)
		{
			(*m_ptr)--;
		}
		m_ptr = ptr.m_ptr;
		(*m_ptr)++;
	}
	void operator=(void * ptr)
	{
		if (ptr == nullptr)
		{
			(*m_ptr)--;
			m_ptr = nullptr;
		}
	}
	_ptr<T> * GetData()
	{
		return  m_ptr;
	}
	~full_ptr()
	{ 
		if (m_ptr != nullptr)
		{
			(*m_ptr)--;
		}
	}
private:
	_ptr<T> * m_ptr;
};