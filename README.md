# full_ptr
 完美智能指针
         今天废话不多说，直接进入主题。

full_ptr制作背景：C++auto_ptr指针不做解释，shader_ptr是引用计数智能指针，它可以让你帮助你托管你的内存，但在游循环引用的地方比如循环链表它就会失效，需要利用week_ptr进行配合使用，相当麻烦和不智能，因此作者兴趣之作衍生出了full_ptr来解决shader_ptr的问题。

**使用并介绍功能：

这个是测试类，用来给full_ptr测试使用。

class test {
public:
	test(char c):c(c) {}
	void show()
	{
		std::cout << "调用了打印函数:" << c << std::endl;
	}
	~test()
	{
		std::cout << "删除:" << c << std::endl;
	}
private:
	char c;
};

创建full指针a并保存test("a"),然后b也保存a指向的空间，c保存test("c")，然后开始。

c也保存a指向的空间，这时test("c")没有被其他full引用会被释放，现在3个full都指向了test("a")。

a和b指针full都打印一下是一样的，然后把a赋值为空没有回test("a"),把b赋值为空也没有回收test("a")，最后函数执行完后回收了c后也回收了test("a")。

**循环链表示例：

这段代码是一个简单的链表结构

template<typename T>
class p1
{
public:
	p1(full_ptr<T> & data) :_data(&data)
	{
	}
	p1<T> * next;
	full_ptr<T> * _data;
};

创建一个full并赋值test('a'),然后创建两个结点f1和f2都保存full指针，并将f1和f2互相引用，然后打印一下内容，最后函数执行结束后test('a')被回收了。这里才是本次文章的重点，如果这里换成shader_ptr是不能回收test('a')的。

**实现原理：

这段是核心代码，这个类用来保存需要托管的堆空间对象，它有一个count引用计数，用来做被引用计数，当计数为0虚构所保存的堆空间，同时也虚构自己。

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

father_ptr其实没什么用，用途就是继承它的类才可以使用_ptr。

class father_ptr
{
public:
	template<typename T>
	class _ptr
	{
	    。。。
	};
};

任何解决shader的问题光靠_ptr肯定不行的，因此就多了一个维护类full_ptr。

在给full赋值的时候都会new一个_ptr并让它来保存传进来的空间，然后full来维护_ptr

当出现另外一个full引用自己(full1 = full2)操作时，full2会将_ptr引用的空间也给full1的_ptr引用

不管哪个full自己被回收了都会在构造函数里面给_ptr进行计数减1，就这样，就算外面结构是网状的，只要有一个full被删除都会使_ptr减1，直到_ptr为0时代表没有full维护它了自己回收了自己。

template<typename T>
class full_ptr : public father_ptr
{
public:
    /*4个构造函数，这些都是赋值使用的
      每个构造函数里都会new_ptr进行托管*/
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
    /*重载->用来可以直接使用full_ptr使用被保存的对象*/
	T * operator->()
	{
		return &m_ptr->GetData();
	}
    /*引用另外一个full所引用的值*/
	void operator=(full_ptr & ptr)
	{
		if (m_ptr != nullptr)
		{
			(*m_ptr)--;
		}
		m_ptr = ptr.m_ptr;
		(*m_ptr)++;
	}
    /*如果赋值为nullptr就将_ptr计数减1*/
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
    /*删除*/自己也将_ptr计数减1*/
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

**完整源码：

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
