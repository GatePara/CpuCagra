// Last Update: 2024-10-29
// Author: Tong Wu
// Description: Graph data structure for nearest neighbor search
#pragma once

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <memory>
#include <vector>
#include <iostream>
#include "memory.hpp"
#include "assert.h"

namespace cpupg
{
  struct GraphInfo
  {
    int N;
    int R;
    int R_INIT;
    int R_KNNG;

    void print()
    {
      std::cout << "N: " << N << std::endl;
      std::cout << "R: " << R << std::endl;
      std::cout << "R_INIT: " << R_INIT << std::endl;
      std::cout << "R_KNNG: " << R_KNNG << std::endl;
    }
  };
  constexpr int EMPTY_ID = -1;

  template <typename id_t>
  struct Graph
  {
    int N, K;

    id_t *data = nullptr;

    std::vector<int> eps;

    // const int graph_po;

    Graph()
    {
      N = 0;
      K = 0;
    }

    // Graph(id_t *edges, int N, int K) : N(N), K(K), data(edges) {}

    Graph(int N, int K)
    {
      init(N, K);
    }

    Graph(const Graph &g) : Graph(g.N, g.K)
    {
      this->eps = g.eps;
      for (int i = 0; i < N; ++i)
      {
        for (int j = 0; j < K; ++j)
        {
          at(i, j) = g.at(i, j);
        }
      }
    }

    void init(int N, int K)
    {
      assert(N > 0);
      assert(K > 0);
      alloc2M((void **)&data, (size_t)N * K * sizeof(id_t), -1);
      this->K = K;
      this->N = N;
      // graph_po = K / 16;
    }

    void destory()
    {
      if (data != nullptr)
      {
        free(data);
        data = nullptr;
      }
    }

    ~Graph()
    {
      destory();
    }

    const int *edges(int u) const { return data + K * u; }

    int *edges(int u) { return data + K * u; }

    id_t at(int i, int j) const { return data[i * K + j]; }

    id_t &at(int i, int j) { return data[i * K + j]; }

    void prefetch(int u, int lines) const
    {
      mem_prefetch((char *)edges(u), lines);
    }

    void save(const std::string &filename) const
    {
      // static_assert(std::is_same_v<id_t, int32_t>);
      std::ofstream writer(filename.c_str(), std::ios::binary);
      int nep = eps.size();
      writer.write((char *)&nep, 4);
      writer.write((char *)eps.data(), nep * 4);
      writer.write((char *)&N, 4);
      writer.write((char *)&K, 4);
      writer.write((char *)data, N * K * 4);
      printf("Graph Saving done\n");
    }

    bool loadNsg(const char *filename)
    {
      std::ifstream in(filename, std::ios::binary);
      if (!in.is_open())
      {
        std::cerr << "Open file failed. path = " << filename << std::endl;
        return false;
      }
      unsigned width, ep_;
      in.read((char *)&width, 4);
      in.read((char *)&ep_, 4);
      eps.resize(1);
      eps[0] = ep_;
      if (width != K)
      {
        std::cerr << "Graph does not match the current index." << std::endl;
        return false;
      }
      size_t nd = 0;
      size_t cc = 0;
      while (!in.eof())
      {
        unsigned edge_num;
        in.read((char *)&edge_num, 4);
        if (in.eof())
          break;
        nd++;
        cc += edge_num;
        if (edge_num > width)
        {
          edge_num = width;
          std::cout << "Node id: " << nd << " has " << edge_num << " edges, only the first " << width << " edges are read." << std::endl;
        }
        std::vector<unsigned> tmp(edge_num);
        in.read((char *)tmp.data(), edge_num * sizeof(unsigned));
        for (unsigned i = 0; i < edge_num; i++)
        {
          at(nd - 1, i) = tmp[i];
        }
      }
      cc /= nd;
      // output some graph information
      std::cout << "Graph width: " << width << ", ep: " << ep_ << ", nd: " << nd << ", cc: " << cc << std::endl;
      return true;
    }

    void loadKnng(const char *filename)
    {
      // knng format
      // k(usigned 4B),vector(unsigned 4B * k),k,vector...
      std::ifstream in(filename, std::ios::binary);
      if (!in.is_open())
      {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
      }

      unsigned k;
      in.read(reinterpret_cast<char *>(&k), sizeof(unsigned));
      // 获取文件大小并计算元素数量
      in.seekg(0, std::ios::end);
      size_t fsize = static_cast<size_t>(in.tellg());
      size_t num = (fsize) / ((k + 1) * sizeof(unsigned)); // 每组 k 个邻居 + 1 个占位符
      in.seekg(sizeof(unsigned), std::ios::beg);           // 重置到第一个记录的开始位置

      destory();

      init(num, k); // 初始化图

      for (size_t i = 0; i < num; i++)
      {
        std::vector<unsigned> tmp(k);
        unsigned id_placeholder;
        in.read(reinterpret_cast<char *>(&id_placeholder), sizeof(unsigned)); // 跳过占位符
        in.read(reinterpret_cast<char *>(tmp.data()), k * sizeof(unsigned));  // 读取 k 个邻居
        for (unsigned j = 0; j < k; j++)
        {
          at(i, j) = tmp[j];
        }
      }

      in.close();
    }

    void loadKnngFbin(const char *filename)
    {
      // fbin format
      // num(usigned 4B),k(unsigned 4B),vector(unsigned 4B * num * k)
      std::ifstream in(filename, std::ios::binary);
      if (!in.is_open())
      {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
      }
      destory(); // 释放原有内存

      unsigned num, k;
      in.read(reinterpret_cast<char *>(&num), sizeof(unsigned));
      in.read(reinterpret_cast<char *>(&k), sizeof(unsigned));
      init(num, k);                                                                        // 初始化图
      in.read(reinterpret_cast<char *>(data), (size_t)num * (size_t)k * sizeof(unsigned)); // 读取所有邻居
      in.close();
    }

    void saveKnng(const char *filename)
    {
      // knng format
      // k(usigned 4B),vector(unsigned 4B * k),k,vector...

      std::ofstream out(filename, std::ios::binary);
      if (!out.is_open())
      {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        exit(1);
      }

      unsigned k = K;

      for (int32_t i = 0; i < N; i++)
      {
        int32_t *edge = edges(i);
        out.write(reinterpret_cast<const char *>(&k), sizeof(unsigned));
        out.write(reinterpret_cast<const char *>(edge), k * sizeof(unsigned));
      }

      out.close();
    }

    void debug(int i)
    {
      for (int j = 0; j < K; j++)
      {
        std::cout << at(i, j) << " ";
      }
      std::cout << std::endl;
    }
  };

} // namespace npuanns