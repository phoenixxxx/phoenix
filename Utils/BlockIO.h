#pragma once

#include "Types.h"
#include <fstream>
#include <vector>

namespace Phoenix
{
	class BlockWriter
	{
	public:
		BlockWriter(cstr_t filePath)
		{
			mStream.open(filePath, std::ios::out | std::ios::binary);
		}

		uint32_t StartBlock(uint32_t Mark)
		{
			std::streamoff BlockStartPos = mStream.tellp();
			Write(Mark); // 4bytes for the chunckID
			Write(Mark); // 4bytes for the Block size (written in EndBlock)
			return (uint32_t)BlockStartPos;
		}

		void EndBlock(uint32_t Pos)
		{
			std::streamoff curpos = mStream.tellp();
			std::streamoff size = curpos - Pos;
			mStream.seekp(Pos + sizeof(uint32_t));			// Move pass the chunckID
			mStream.write((char *)&size, sizeof(uint32_t)); // write the size
			mStream.seekp(curpos);							// go back to the current position
		}

		void Write(const stdstr_t &str)
		{
			Write(uint32_t(str.size()));
			if (str.size())
			{
				WriteWithCount(&str[0], (uint32_t)str.size());
			}
		}

		template <typename type>
		void Write(const std::vector<type> &vec)
		{
			size_t count = vec.GetCount();
			mStream.write((const char *)&count, sizeof(count));
			if (count > 0)
				mStream.write((const char *)&vec[0], count * sizeof(type));
		}

		template <typename type>
		void Write(const type &data)
		{
			mStream.write((const char *)&data, sizeof(type));
		}

		template <typename type>
		void WriteWithCount(type *data, uint32_t Number)
		{
			mStream.write((char *)data, sizeof(type) * Number);
		}
		template <typename type>
		void WriteWithSize(type *data, uint32_t size)
		{
			mStream.write((char *)data, size);
		}
		void WriteBlock(uint32_t Mark, uint32_t Size)
		{
			Write(Mark);
			Write(Size);
		};

	private:
		std::ofstream mStream;
	};

	class BlockReader
	{
	public:
		struct sBlock
		{
			uint32_t Type;
			uint32_t Size;
			uint32_t StartPos;
			uint32_t EndPos;
		};

	private:
		bool ReadBlockHeader(sBlock &c)
		{
			c.Type = 0;
			mStream.read((char *)&c.Type, sizeof(uint32_t) * 2);
			c.StartPos = (uint32_t)mStream.tellg();
			c.EndPos = c.StartPos + (c.Size - (sizeof(uint32_t) * 2)); // the size includes the header size, remove it because StartPos already skipped it
			return !EndOfFile();
		}

	public:
		BlockReader(cstr_t filePath)
		{
			mStream.open(filePath, std::ios::in | std::ios::binary);
		}

		bool EndOfFile()
		{
			return mStream.eof();
		}

		bool PeekBlock(sBlock &block)
		{
			bool success = ReadBlockHeader(block);
			mStream.seekg(-int(sizeof(uint32_t) * 2), std::ios::cur);
			return success; // if end of file
		}

		bool ReadUntilBlock(sBlock &Block, uint32_t Blocktype)
		{
			while (true)
			{
				if (!PeekBlock(Block))
					return false;
				if (EndOfFile())
					return false;

				if (Block.Type == Blocktype)
					return true;

				SkipBlock(Block);
			}
			return false;
		}

		void SkipBlock(const sBlock &c)
		{
			// in the case when we reached the end of the file, seekg will put us right back BEFORE the eof position (-1)
			// which is a problem the we expect "skipping" the last block should set the EoF bit since we are passed the last
			// block in the file
			if (!EndOfFile())
				mStream.seekg(c.EndPos);
		}

		void MoveToBlock(const sBlock &c)
		{
			// necessary because we might reach the end of the file and when we go back
			// in the file, we want to make sure we unset the EoF bit
			mStream.clear();
			mStream.seekg(c.StartPos);
		}

		void MoveToBegining()
		{
			// necessary because we might reach the end of the file and when we go back
			// in the file, we want to make sure we unset the EoF bit
			mStream.clear();
			mStream.seekg(0, std::ios::beg);
		}

		template <typename type>
		void Read(type &data)
		{
			mStream.read((char *)&data, sizeof(type));
		}

		void Read(stdstr_t &str)
		{
			uint32_t strSize = 0;
			Read(strSize);
			if (strSize)
			{
				str.resize(strSize);
				ReadWithCount(&str[0], strSize);
			}
		}

		template <typename type>
		void Read(std::vector<type> &vec)
		{
			uint32_t count = 0;
			mStream.read((char *)&count, sizeof(count));
			if (count)
			{
				vec.Allocate(count);
				mStream.read((char *)&vec[0], count * sizeof(type));
			}
		}

		template <typename type>
		void ReadWithCount(type *data, uint32_t Number)
		{
			mStream.read((char *)data, sizeof(type) * Number);
		}

		template <typename type>
		void ReadWithSize(type *data, int Size)
		{
			mStream.read((char *)data, Size);
		}

	private:
		std::ifstream mStream;
	};
}
