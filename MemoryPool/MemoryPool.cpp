#include <iostream>
#include <map>


#pragma pack(push,1)
struct MemoryBlock
{
	//change pointers to the relatives addresses
	MemoryBlock* mp_nextStructure = nullptr;
	MemoryBlock* mp_prevStructure = nullptr;
	char* mp_memoryBlock = nullptr;
	unsigned int m_memoryBlockSize = 0;
	bool m_allocated = false;
};
#pragma pack(pop)

class MemoryPool
{
private:
	unsigned int m_poolSize = 0;
	char* mp_startOfPool = nullptr;
	char* mp_endOfPool = nullptr;
	MemoryBlock* mp_memoryBlocks = nullptr;
	unsigned int m_numberOfMemoryBlocks = 0;
	unsigned int m_numberOfMemoryBlocksStructure = 5;

	void ReallocateMemoryForStructures() 
	{
		const unsigned int countToAdd = 10;
		const unsigned int newCount = m_numberOfMemoryBlocksStructure + countToAdd;
		MemoryBlock* newMemoryBlocks = new MemoryBlock[newCount];
		long long int memoryShift = reinterpret_cast<uint64_t>(mp_memoryBlocks) - reinterpret_cast<uint64_t>(newMemoryBlocks);
		for (unsigned int i = 0; i < m_numberOfMemoryBlocks; ++i)
		{
			*(newMemoryBlocks+i) = *(mp_memoryBlocks+i);

			uint64_t index = 0;
			if ((mp_memoryBlocks + i)->mp_nextStructure != nullptr)
			{
				index = StructureIndexFromAddress((mp_memoryBlocks + i)->mp_nextStructure);
				(newMemoryBlocks + i)->mp_nextStructure = newMemoryBlocks + index;
			}
				
			if ((mp_memoryBlocks + i)->mp_prevStructure != nullptr)
			{
				index = StructureIndexFromAddress((mp_memoryBlocks + i)->mp_prevStructure);
				(newMemoryBlocks + i)->mp_prevStructure = newMemoryBlocks + index;
			}
		}
		delete[] mp_memoryBlocks;
		mp_memoryBlocks = newMemoryBlocks;
		m_numberOfMemoryBlocksStructure = newCount;
	}

	uint64_t StructureIndexFromAddress(const MemoryBlock* const ip_memoryBlock) const
	{
		if (ip_memoryBlock == nullptr)
			return 0;

		return (reinterpret_cast<uint64_t>(ip_memoryBlock) - reinterpret_cast<uint64_t>(mp_memoryBlocks)) / sizeof(MemoryBlock);
	}

	MemoryBlock* StructureAddressFromIndex(MemoryBlock* const ip_from, const uint64_t index) const
	{
		if (ip_from == nullptr)
			return nullptr;
		return ip_from + index ;
	}

	void MargeNotAllocatedBlocks(MemoryBlock* const ip_memoryBlock)
	{

		MemoryBlock* toDelete = ip_memoryBlock;

		if (toDelete->mp_nextStructure != nullptr)
			if (!((toDelete->mp_nextStructure)->m_allocated))
			{
				toDelete->m_memoryBlockSize += (toDelete->mp_nextStructure)->m_memoryBlockSize;
				DeleteStructure(toDelete->mp_nextStructure);
			}

		if (toDelete->mp_prevStructure != nullptr)
			if (!((toDelete->mp_prevStructure)->m_allocated))
			{
				(toDelete->mp_prevStructure)->m_memoryBlockSize += toDelete->m_memoryBlockSize;
				DeleteStructure(toDelete);
				//toDelete--;
			}
	}

	void DeleteStructure(MemoryBlock* const ip_memoryBlock)
	{
		//if (!ip_memoryBlock->m_allocated)
	
		MemoryBlock* previousBlock = ip_memoryBlock->mp_prevStructure;
		MemoryBlock* nextBlock = ip_memoryBlock->mp_nextStructure;

		if (nextBlock != nullptr)
			nextBlock->mp_prevStructure = previousBlock;

		if (previousBlock != nullptr)
			previousBlock->mp_nextStructure = (nextBlock != nullptr) ? nextBlock : nullptr;

		uint64_t index = 0;
		for (uint64_t memoryBlockIndex = StructureIndexFromAddress(ip_memoryBlock);
			memoryBlockIndex < m_numberOfMemoryBlocks - 1; memoryBlockIndex++)
		{
			mp_memoryBlocks[memoryBlockIndex] = mp_memoryBlocks[memoryBlockIndex + 1];
		}
		m_numberOfMemoryBlocks--;

		(mp_memoryBlocks + m_numberOfMemoryBlocks)->mp_nextStructure = nullptr;
		(mp_memoryBlocks + m_numberOfMemoryBlocks)->mp_prevStructure = nullptr;
		(mp_memoryBlocks + m_numberOfMemoryBlocks)->mp_memoryBlock = nullptr;
		(mp_memoryBlocks + m_numberOfMemoryBlocks)->m_allocated= false;

		index = StructureIndexFromAddress(ip_memoryBlock);
		for (uint64_t memoryBlockIndex = 0;
			memoryBlockIndex < m_numberOfMemoryBlocks; memoryBlockIndex++)
		{
			if ((mp_memoryBlocks + memoryBlockIndex)->mp_nextStructure != nullptr)
			{
				if (StructureIndexFromAddress((mp_memoryBlocks + memoryBlockIndex)->mp_nextStructure) >= index)
					(mp_memoryBlocks + memoryBlockIndex)->mp_nextStructure -= 1;
			}
			if ((mp_memoryBlocks + memoryBlockIndex)->mp_prevStructure != nullptr)
			{
				if (StructureIndexFromAddress((mp_memoryBlocks + memoryBlockIndex)->mp_prevStructure) >= index)
					(mp_memoryBlocks + memoryBlockIndex)->mp_prevStructure -= 1;
			}

		}
	}



public:

	void StructureDetails() const
	{
		if (mp_memoryBlocks == nullptr)
		{
			std::cout << "No structure" << std::endl;
			return;
		}
		std::cout << "#\t" << "Prev stuct addr\t" << "Own adress\t" << "Next stuct addr\t" << "MemoryBlock addr" << '\t' << std::endl;
		
		std::cout << "------------------------------------------------------------------------" << std::endl;
		for (unsigned int i = 0; i < m_numberOfMemoryBlocks; i++)
		{
			std::cout << i << '\t'
				<< reinterpret_cast<uint64_t>((mp_memoryBlocks + i)->mp_prevStructure) << '\t'
				<< reinterpret_cast<uint64_t>((mp_memoryBlocks + i)) << '\t'
				<< reinterpret_cast<uint64_t>((mp_memoryBlocks + i)->mp_nextStructure) << '\t'
				<< reinterpret_cast<uint64_t>((mp_memoryBlocks + i)->mp_memoryBlock) << '\t'
				<< std::endl;
		}
		std::cout << "------------------------------------------------------------------------" << std::endl;
		std::cout << std::endl;
	}

	void StructureDetailsIndexes() const
	{
		if (mp_memoryBlocks == nullptr)
		{
			std::cout << "No structure" << std::endl;
			return;
		}

		std::cout << "Prev\t" << "Own\t" << "Next\t" << "MemoryBlock addr" << '\t' << std::endl;
		std::cout << "-----------------------------------------" << std::endl;
		for (unsigned int i = 0; i < m_numberOfMemoryBlocks; i++)
		{
			std::cout 
				<< StructureIndexFromAddress((mp_memoryBlocks + i)->mp_prevStructure) << '\t'
				<< i << '\t'
				<< StructureIndexFromAddress((mp_memoryBlocks + i)->mp_nextStructure) << '\t'
				<< reinterpret_cast<uint64_t>((mp_memoryBlocks + i)->mp_memoryBlock) << '\t'
				<< std::endl;
		}
		std::cout << "-----------------------------------------" << std::endl;
		std::cout << std::endl;
	}

	explicit MemoryPool(const unsigned int i_PoolSize) : m_poolSize(i_PoolSize)
	{
		if (i_PoolSize == 0)
			return;

		mp_startOfPool = new char[m_poolSize];
		mp_endOfPool = mp_startOfPool + m_poolSize;

		mp_memoryBlocks = new MemoryBlock[m_numberOfMemoryBlocksStructure];
		m_numberOfMemoryBlocks++;

		mp_memoryBlocks[0].mp_memoryBlock = mp_startOfPool;
		mp_memoryBlocks[0].m_memoryBlockSize = m_poolSize;
	}
	~MemoryPool()
	{
		delete[] mp_startOfPool;
		mp_startOfPool = nullptr;

		delete[] mp_memoryBlocks;
		mp_memoryBlocks = nullptr;
	}


	void* Allocate(const unsigned int i_sizeOfMemory) 
	{

		if (mp_startOfPool == nullptr)
			return nullptr;
		if (i_sizeOfMemory > m_poolSize)
			return nullptr;
		
		if (100 * m_numberOfMemoryBlocks / m_numberOfMemoryBlocksStructure >= 80)
			ReallocateMemoryForStructures();

		for (MemoryBlock* currentBlock = mp_memoryBlocks; currentBlock != nullptr; currentBlock = currentBlock->mp_nextStructure)
		{
			if (currentBlock->m_memoryBlockSize >= i_sizeOfMemory && !(currentBlock->m_allocated))
			{
				if (currentBlock->m_memoryBlockSize == i_sizeOfMemory)
				{
					currentBlock->m_allocated = true;
					return currentBlock->mp_memoryBlock;
				}
	
				MemoryBlock* nextBlock = mp_memoryBlocks + m_numberOfMemoryBlocks;
				nextBlock->m_memoryBlockSize = currentBlock->m_memoryBlockSize - i_sizeOfMemory;
				nextBlock->mp_prevStructure = currentBlock;
				nextBlock->mp_nextStructure = currentBlock->mp_nextStructure;
				nextBlock->mp_memoryBlock = currentBlock->mp_memoryBlock + i_sizeOfMemory;
				++m_numberOfMemoryBlocks;

				if (nextBlock->mp_nextStructure != nullptr)
					(nextBlock->mp_nextStructure)->mp_prevStructure = nextBlock;

				currentBlock->mp_nextStructure = nextBlock;
				currentBlock->m_memoryBlockSize = i_sizeOfMemory;
				currentBlock->m_allocated = true;

				return currentBlock->mp_memoryBlock;
			}
		}

		return nullptr;
	}

	bool Deallocate(void *iop_toDelete) 
	{
		if (iop_toDelete == nullptr)
			return false;
		
		if (iop_toDelete < mp_startOfPool || iop_toDelete > mp_endOfPool)
			return false;

		for (MemoryBlock* currentBlock = mp_memoryBlocks; currentBlock != nullptr; currentBlock = currentBlock->mp_nextStructure)
		{
			if (currentBlock->mp_memoryBlock == iop_toDelete)
			{
				currentBlock->m_allocated = false;
				MargeNotAllocatedBlocks(currentBlock);
				return true;
			}
		}
		return false;
	}



	void MemoryUsage() const
	{
		if (mp_startOfPool == nullptr)
		{
			std::cout << "Pool is not holding memory" << std::endl;
			return;
		}
		
		std::cout << "Pool is handling " << m_poolSize << "bytes from adress " << reinterpret_cast<uint64_t>(mp_startOfPool) << std::endl;

		std::cout << "Absolute addr\tRelative addr\tBlock size\tAllocated" << std::endl;
		std::cout << "---------------------------------------------------------" << std::endl;
		for (MemoryBlock* block = mp_memoryBlocks; block != nullptr; block = block->mp_nextStructure)
		{
			std::cout << reinterpret_cast<uint64_t>(block->mp_memoryBlock) << '\t'
				<< reinterpret_cast<uint64_t>(block->mp_memoryBlock) - reinterpret_cast<uint64_t>(mp_startOfPool) << "\t\t"
				<< block->m_memoryBlockSize << "\t\t"
				<< (block->m_allocated ? "yes" : "no") << std::endl;
		}
		std::cout << "---------------------------------------------------------" << std::endl << std::endl;
	}
};

int main()
{
	MemoryPool pool(200);
	pool.MemoryUsage();

	char* block0 = reinterpret_cast<char*>(pool.Allocate(10));
	char* block1 = reinterpret_cast<char*>(pool.Allocate(15));
	char* block2 = reinterpret_cast<char*>(pool.Allocate(15));
	char* block3 = reinterpret_cast<char*>(pool.Allocate(10));
	char* block4 = reinterpret_cast<char*>(pool.Allocate(10));

	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();

	pool.Deallocate(reinterpret_cast<void*>(block1));
	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();
	pool.Deallocate(reinterpret_cast<void*>(block3));
	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();
	pool.Deallocate(reinterpret_cast<void*>(block2));
	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();
	
	char* block5 = reinterpret_cast<char*>(pool.Allocate(12));
	char* block6 = reinterpret_cast<char*>(pool.Allocate(100));
	pool.MemoryUsage();

	char* block7 = reinterpret_cast<char*>(pool.Allocate(15));
	char* block8 = reinterpret_cast<char*>(pool.Allocate(13));
	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();

	pool.Deallocate(reinterpret_cast<void*>(block5));
	pool.Deallocate(reinterpret_cast<void*>(block8));
	pool.Deallocate(reinterpret_cast<void*>(block7));
	pool.MemoryUsage();
	//pool.StructureDetailsIndexes();

	pool.Deallocate(reinterpret_cast<void*>(block0));
	pool.Deallocate(reinterpret_cast<void*>(block4));
	pool.Deallocate(reinterpret_cast<void*>(block6));
	pool.MemoryUsage();

	return 0;
}

