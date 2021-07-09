#include <iostream>
#include <map>

class MemoryPool
{
public:
	explicit MemoryPool(unsigned int i_PoolSize) : m_poolSize(i_PoolSize)
	{
		if (i_PoolSize == 0)
			return;
		mp_startOfPool = new char[m_poolSize];
		mp_endOfPool = mp_startOfPool + m_poolSize;
	}
	~MemoryPool()
	{
		delete[] mp_startOfPool;
		mp_startOfPool = nullptr;
	}

	void* allocate(unsigned int i_sizeOfMemory) 
	{
		if (mp_startOfPool == nullptr)
			return nullptr;
		if (i_sizeOfMemory > m_poolSize)
			return nullptr;
		if (m_numberOfBlocks == 0)
		{
			m_memoryBlocks[mp_startOfPool] = i_sizeOfMemory;
			++m_numberOfBlocks;
			return reinterpret_cast<void*>(mp_startOfPool);
		}

		bool blockFount = false;
		auto blocksIterator = m_memoryBlocks.begin();
		auto lastBlock = --m_memoryBlocks.end();
		while (blocksIterator != lastBlock)
		{
			char* currentBlockEndAt = blocksIterator->first + blocksIterator->second;
			blocksIterator++;
			char* nextBlockBeganAt = blocksIterator->first;

			if (nextBlockBeganAt - currentBlockEndAt >= i_sizeOfMemory)
			{
				m_memoryBlocks[currentBlockEndAt] = i_sizeOfMemory;
				++m_numberOfBlocks;
				return reinterpret_cast<void*>(currentBlockEndAt);
			}
		}
		char* currentBlockEndAt = blocksIterator->first + blocksIterator->second;
		if (mp_endOfPool - currentBlockEndAt >= i_sizeOfMemory)
		{
			m_memoryBlocks[currentBlockEndAt] = i_sizeOfMemory;
			++m_numberOfBlocks;
			return reinterpret_cast<void*>(currentBlockEndAt);
		}

		return nullptr;
	}

	bool deallocate(void *iop_toDelete) 
	{
		if (iop_toDelete == nullptr)
			return false;
		
		if (iop_toDelete < mp_startOfPool || iop_toDelete > mp_endOfPool)
			return false;

		if (auto blockIterator = m_memoryBlocks.find(reinterpret_cast<char*>(iop_toDelete));
			blockIterator != m_memoryBlocks.end())
		{
			m_memoryBlocks.erase(blockIterator);
			return true;
		}

		return false;
	}

	void memoryUsage() const
	{
		if (mp_startOfPool == nullptr)
		{
			std::cout << "Pool is not holding memory" << std::endl;
			return;
		}
		
		std::cout << "Pool is handling " << m_poolSize << "bytes from adress " << reinterpret_cast<uint64_t>(mp_startOfPool) << std::endl;

		if (m_numberOfBlocks == 0) 
		{
			std::cout << "Whole pool is free" << std::endl;
			return;
		}

		std::cout << "Block size\tAbsolute addr\tRelative addr" << std::endl;
		for (auto memoryBlock : m_memoryBlocks) 
		{
			std::cout << memoryBlock.second << "\t\t"
					<< reinterpret_cast<uint64_t>(memoryBlock.first) << '\t'
					<< reinterpret_cast<uint64_t>(memoryBlock.first) - reinterpret_cast<uint64_t>(mp_startOfPool) << std::endl;
		}
		std::cout << std::endl;
	}


private:
	char* mp_startOfPool = nullptr;
	char* mp_endOfPool = nullptr;
	unsigned int m_poolSize = 0;
	//pointer on memmory and its size
	std::map<char*, unsigned int> m_memoryBlocks;
	//now it is a number of pairs in map - 2
	unsigned int m_numberOfBlocks = 0;
};




int main()
{
	MemoryPool pool(200);
	char* block1 = reinterpret_cast<char*>(pool.allocate(10));
	char* block2 = reinterpret_cast<char*>(pool.allocate(15));
	char* block3 = reinterpret_cast<char*>(pool.allocate(15));
	pool.memoryUsage();

	pool.deallocate(reinterpret_cast<void*>(block2));
	pool.memoryUsage();

	char* block4 = reinterpret_cast<char*>(pool.allocate(10));
	char* block5 = reinterpret_cast<char*>(pool.allocate(10));
	char* block6 = reinterpret_cast<char*>(pool.allocate(5));
	pool.memoryUsage();

	char* block7 = reinterpret_cast<char*>(pool.allocate(150));

	pool.deallocate(reinterpret_cast<void*>(block1));
	pool.deallocate(reinterpret_cast<void*>(block2)); //false
	pool.deallocate(reinterpret_cast<void*>(block3));
	pool.deallocate(reinterpret_cast<void*>(block4));
	pool.deallocate(reinterpret_cast<void*>(block5));
	pool.deallocate(reinterpret_cast<void*>(block6));
	pool.memoryUsage();

	char* block8 = reinterpret_cast<char*>(pool.allocate(50));
	pool.memoryUsage();


	return 0;
}

