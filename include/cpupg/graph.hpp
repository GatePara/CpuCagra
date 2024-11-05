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
#include <cassert>
#include "memory.hpp"

namespace cpupg
{
  struct GraphInfo
  {
    uint64_t N;
    uint64_t R;
    uint64_t R_INIT;
    uint64_t R_KNNG;

    void print()
    {
      std::cout << "N: " << N << std::endl;
      std::cout << "R: " << R << std::endl;
      std::cout << "R_INIT: " << R_INIT << std::endl;
      std::cout << "R_KNNG: " << R_KNNG << std::endl;
    }
  };

  constexpr int EMPTY_ID = -1;
  template <typename id_t = int32_t>
  struct Graph
  {
    id_t N;
    uint64_t K;

    id_t *data = nullptr;

    std::vector<id_t> eps;

    // const int graph_po;

    Graph()
    {
      N = 0;
      K = 0;
    }

    // Graph(id_t *edges, int N, int K) : N(N), K(K), data(edges) {}

    Graph(id_t N, uint64_t K)
    {
      init(N, K);
    }

    Graph(const Graph &g) : Graph(g.N, g.K)
    {
      this->eps = g.eps;
      for (id_t i = 0; i < N; ++i)
      {
        for (uint64_t j = 0; j < K; ++j)
        {
          at(i, j) = g.at(i, j);
        }
      }
    }

    void init(id_t N, uint64_t K)
    {
      assert(N > 0);
      assert(K > 0);
      alloc2M((void **)&data, N * K * sizeof(id_t), -1);
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

    const id_t *edges(id_t u) const { return data + K * u; }

    id_t *edges(id_t u) { return data + K * u; }

    id_t at(id_t i, uint64_t j) const { return data[i * K + j]; }

    id_t &at(id_t i, uint64_t j) { return data[i * K + j]; }

    void prefetch(id_t u, int lines) const
    {
      mem_prefetch((char *)edges(u), lines);
    }

    void save(const std::string &filename) const
    {
      static_assert(std::is_same_v<id_t, int32_t>);
      std::ofstream writer(filename.c_str(), std::ios::binary);
      int nep = eps.size();
      writer.write((char *)&nep, 4);
      writer.write((char *)eps.data(), nep * 4);
      writer.write((char *)&N, 4);
      writer.write((char *)&K, 4);
      writer.write((char *)data, N * K * 4);
      printf("Graph Saving done\n");
    }

    void loadNsg(const char *filename, id_t baseN)
    {
      std::ifstream in(filename, std::ios::binary);
      if (!in.is_open())
      {
        std::cerr << "Open file failed. path = " << filename << std::endl;
        exit(1);
      }
      unsigned width, ep_;
      in.read((char *)&width, 4);
      in.read((char *)&ep_, 4);
      eps.resize(1);
      eps[0] = ep_;
      destory();
      init(baseN, width);
      size_t nd = 0;
      size_t cc = 0;
      while (!in.eof())
      {
        unsigned edge_num;
        in.read((char *)&edge_num, 4);
        if (in.eof())
          break;
        nd++;
        if (nd > N)
        {
          std::cerr << "Error: The number of nodes in the graph is larger than the specified value." << std::endl;
          exit(1);
        }
        cc += edge_num;
        if (edge_num > width)
        {
          std::cerr << "Error: The number of edges in the graph is larger than the specified value." << std::endl;
          exit(1);
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
      in.seekg(0, std::ios::end);
      size_t fsize = static_cast<size_t>(in.tellg());
      size_t num = (fsize) / ((k + 1) * sizeof(unsigned));
      in.seekg(sizeof(unsigned), std::ios::beg);

      destory();
      init(num, k);

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

      for (id_t i = 0; i < N; i++)
      {
        id_t *edge = edges(i);
        out.write(reinterpret_cast<const char *>(&k), sizeof(unsigned));
        out.write(reinterpret_cast<const char *>(edge), k * sizeof(id_t));
      }

      out.close();
    }

    void debug(id_t i)
    {
      for (uint64_t j = 0; j < K; j++)
      {
        std::cout << at(i, j) << " ";
      }
      std::cout << std::endl;
    }
  };

} // namespace npuanns