#ifndef _TUTTLE_PROCESSVISITORS_HPP_
#define _TUTTLE_PROCESSVISITORS_HPP_

#include "ProcessOptions.hpp"
#include <iostream>
#include <vector>
#include <boost/graph/properties.hpp>
#include <boost/graph/visitors.hpp>

namespace tuttle {
namespace host {
namespace core {

struct dfs_compute_visitor : public boost::dfs_visitor<>
{
	public:
		dfs_compute_visitor(const ProcessOptions & options)
			: _options(options)
		{}

		template<class VertexDescriptor, class Graph>
		void initialize_vertex( VertexDescriptor v, Graph& g )
		{
			std::cout << "[PROCESS] initialize_vertex "
			          << get( vertex_properties, g )[v] << std::endl;

			get( vertex_properties, g )[v].processNode()->process(_options);

		}

	private:
		const ProcessOptions & _options;
};


struct dfs_connect_visitor : public boost::dfs_visitor<>
{
	public:
		dfs_connect_visitor(){}

		template<class EdgeDescriptor, class Graph>
		void examine_edge( EdgeDescriptor e, Graph& g )
		{
			std::cout << "examine_edge "
					  << get( vertex_properties, g )[source(e, g)]
					  << " TO "
					  << get( vertex_properties, g )[target(e, g)]
			          << std::endl;

			core::ProcessNode * sourceNode = get( vertex_properties, g )[source(e, g)].processNode();
			core::ProcessNode * targetNode = get( vertex_properties, g )[target(e, g)].processNode();
			targetNode->connect( *sourceNode );
		}
};

} // namespace core
} // namespace host
} // namespace tuttle

#endif
