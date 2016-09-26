// -*- tab-width: 2; indent-tabs-mode: nil; c-basic-offset: 2 -*-
// vi: set et ts=4 sw=2 sts=2:
#include <config.h>

#include <cassert>
#include <cstdlib>

#include <algorithm>
#include <memory>
#include <typeinfo>
#include <utility>

#if HAVE_CXA_DEMANGLE
#include <cxxabi.h>
#endif // #HAVE_CXA_DEMANGLE

#include <dune/corepy/common/common.hh>
#include <dune/corepy/common/dynmatrix.hh>
#include <dune/corepy/common/dynvector.hh>
#include <dune/corepy/common/fmatrix.hh>
#include <dune/corepy/common/fvector.hh>
#include <dune/corepy/common/mpihelper.hh>

#include <dune/grid/common/gridenums.hh>
#include <dune/grid/io/file/vtk/vtkwriter.hh>

#include <dune/corepy/pybind11/pybind11.h>

std::string className ( void *typeInfo )
{
  const auto &types = pybind11::detail::get_internals().registered_types_cpp;
  const auto pred = [ typeInfo ] ( const std::pair< const std::type_index, void * > &t ) { return (t.second == typeInfo); };
  const auto it = std::find_if( types.begin(), types.end(), pred );
  assert( it != types.end() );
  std::string className = it->first.name();
#if HAVE_CXA_DEMANGLE
  int status = 0;
  std::unique_ptr< char[], void (*)( void * ) > demangled( abi::__cxa_demangle( className.c_str(), nullptr, nullptr, &status ), std::free );
  if( (status == 0) && demangled )
    className = demangled.get();
#endif // #if HAVE_CXA_DEMANGLE
  return className;
}

PYBIND11_PLUGIN(_common)
{
  pybind11::module module( "_common" );

  module.def( "className", [] ( pybind11::object o ) {
      return className( pybind11::detail::get_type_info( Py_TYPE( o.ptr() ) ) );
    } );

  Dune::CorePy::registerFieldVector<double>(module, std::make_integer_sequence<int, 10>());
  Dune::CorePy::registerFieldMatrix<double>(module, std::make_integer_sequence<int, 5>());

  Dune::CorePy::registerDynamicVector<double>(module);
  Dune::CorePy::registerDynamicMatrix<double>(module);

  Dune::CorePy::registerCollectiveCommunication(module);

  pybind11::enum_< Dune::CorePy::Reader > reader( module, "reader" );
  reader.value( "dgf", Dune::CorePy::Reader::dgf );
  reader.value( "dgfString", Dune::CorePy::Reader::dgfString );
  reader.value( "gmsh", Dune::CorePy::Reader::gmsh );

  pybind11::enum_< Dune::CorePy::VTKDataType > vtkDataType( module, "DataType" );
  vtkDataType.value( "CellData", Dune::CorePy::VTKDataType::CellData );
  vtkDataType.value( "PointData", Dune::CorePy::VTKDataType::PointData );
  vtkDataType.value( "CellVector", Dune::CorePy::VTKDataType::CellData );
  vtkDataType.value( "PointVector", Dune::CorePy::VTKDataType::PointData );

  pybind11::enum_< Dune::PartitionType > partitionType( module, "PartitionType" );
  partitionType.value( "Interior", Dune::InteriorEntity );
  partitionType.value( "Border", Dune::BorderEntity );
  partitionType.value( "Overlap", Dune::OverlapEntity );
  partitionType.value( "Front", Dune::FrontEntity );
  partitionType.value( "Ghost", Dune::GhostEntity );

  pybind11::enum_< Dune::PartitionIteratorType > partitionIteratorType( module, "PartitionIteratorType" );
  partitionIteratorType.value( "Interior", Dune::Interior_Partition );
  partitionIteratorType.value( "InteriorBorder", Dune::InteriorBorder_Partition );
  partitionIteratorType.value( "Overlap", Dune::Overlap_Partition );
  partitionIteratorType.value( "OverlapFront", Dune::OverlapFront_Partition );
  partitionIteratorType.value( "All", Dune::All_Partition );
  partitionIteratorType.value( "Ghost", Dune::Ghost_Partition );

  pybind11::enum_< Dune::InterfaceType > interfaceType( module, "InterfaceType" );
  interfaceType.value( "InteriorBorder_InteriorBorder", Dune::InteriorBorder_InteriorBorder_Interface );
  interfaceType.value( "InteriorBorder_All", Dune::InteriorBorder_All_Interface );
  interfaceType.value( "Overlap_OverlapFront", Dune::Overlap_OverlapFront_Interface );
  interfaceType.value( "Overlap_All", Dune::Overlap_All_Interface );
  interfaceType.value( "All_All", Dune::All_All_Interface );

  pybind11::enum_< Dune::CommunicationDirection > communicationDirection( module, "CommunicationDirection" );
  communicationDirection.value( "Forward", Dune::ForwardCommunication );
  communicationDirection.value( "Backward", Dune::BackwardCommunication );

  pybind11::enum_<Dune::VTK::OutputType> vtkOutputType(module, "OutputType");
  vtkOutputType.value("ascii", Dune::VTK::OutputType::ascii);
  vtkOutputType.value("base64", Dune::VTK::OutputType::base64);
  vtkOutputType.value("appendedraw", Dune::VTK::OutputType::appendedraw);
  vtkOutputType.value("appendedbase64", Dune::VTK::OutputType::appendedbase64);

  pybind11::enum_< Dune::CorePy::Marker > marker( module, "Marker" );
  marker.value( "coarsen", Dune::CorePy::Marker::Coarsen );
  marker.value( "keep", Dune::CorePy::Marker::Keep );
  marker.value( "refine", Dune::CorePy::Marker::Refine );

  return module.ptr();
}
