#include "external_functions.h"
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <comdef.h>
#include <fstream>

#include <map>
#include <algorithm>
#include "aux_functions.h"
//xml parcers
#include <pugixml.hpp>
using namespace pugi;
//open cascade headers
#include <IGESCAFControl_Writer.hxx>
#include <gp_Pnt.hxx>

#include <TopoDS_Edge.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Wire.hxx>
#include <TopoDS_Shell.hxx>
#include <TopoDS_Solid.hxx>

#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_MakeShell.hxx>
#include <BRepBuilderAPI_MakeShape.hxx>
#include <BRepBuilderAPI_MakeSolid.hxx>

#include <Geom_Curve.hxx>

#include <IGESControl_Controller.hxx> 
#include <IGESControl_Writer.hxx> 
#include <TopoDS_Shape.hxx> 

#include <IVtkTools_ShapeDataSource.hxx>
#include <vtkAutoInit.h>
#include <vtkRenderer.h>
#include <vtkRenderWindow.h>
#include <TopoDS_Shape.hxx>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkPolyDataMapper.h>

VTK_MODULE_INIT(vtkRenderingOpenGL2)
VTK_MODULE_INIT(vtkInteractionStyle)
//#include "vtk_vis.cpp"

namespace fs = std::filesystem;
void visualize_figure(TopoDS_Shape* to_vizualize)
{
    vtkNew<vtkRenderWindow> renwin;
    vtkNew<vtkRenderer> ren;
    renwin->AddRenderer(ren);

    vtkNew<vtkInteractorStyleTrackballCamera> istyle;
    vtkNew<vtkRenderWindowInteractor> iren;
    //
    iren->SetRenderWindow(renwin);
    iren->SetInteractorStyle(istyle);

    //data -> vtkPolyData
    vtkNew<IVtkTools_ShapeDataSource> occSource;
    occSource->SetShape(new IVtkOCC_Shape(*to_vizualize));

    vtkNew< vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(occSource->GetOutputPort());
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    ren->AddActor(actor);

    renwin->Render();
    iren->Start();
}

void run_landxml2iges(const char* dir_path)
{
    //fs::path file_path = dir_path;
    pugi::xml_document landxml_file;
    pugi::xml_parse_result landxml_file_result = landxml_file.load_file(dir_path);

    if (landxml_file_result)
    {
        pugi::xml_node surfaces = landxml_file.child("LandXML").child("Surfaces");
        for (pugi::xml_node one_surface = surfaces.child("Surface"); one_surface; one_surface = one_surface.next_sibling("Surface"))
        {
            const char* surface_name = one_surface.attribute("name").value();
            pugi::xml_node pnts = one_surface.child("Definition").child("Pnts");
            pugi::xml_node faces = one_surface.child("Definition").child("Faces");
            std::map<int, int> old_index2new_index;
            
            //for (pugi::xml_node point = pnts.child("P"); point; point = point.next_sibling("P")
            std::vector <gp_Pnt> temp_points;
            int counter_points = 0;
            float z_min = 5000000.0;
            for (pugi::xml_node one_point : pnts.children("P"))
            {
                int old_point_index = ::atoi(one_point.attribute("id").value());

                std::vector<float> current_point;
                std::stringstream ss(one_point.first_child().value());
                std::string item;

                while (getline(ss, item, ' ')) {
                    current_point.push_back(::atof(item.c_str()));
                }
                gp_Pnt new_point(current_point.at(0), current_point.at(1), current_point.at(2));
                if (current_point.at(2) < z_min) z_min = current_point.at(2);
                temp_points.push_back(new_point);
                //points_list[counter_points] = new_point;
                old_index2new_index.insert(std::pair<int, int>{old_point_index, counter_points});
                counter_points++;
            }
            z_min -= 100.0;

            BRep_Builder builder;
            TopoDS_Shell surface_top_shell;
            builder.MakeShell(surface_top_shell);

            std::vector<std::vector<int>> edges_ones;
            std::vector<std::vector<int>> edges_external;

            int triangles_count = 0;
            for (pugi::xml_node one_triangle : faces.children("F"))
            {
                triangles_count++;
                std::vector<int> triangle_points;
                std::stringstream ss(one_triangle.first_child().value());
                std::string item;

                while (getline(ss, item, ' ')) {
                    triangle_points.push_back(atoi(item.c_str()));
                }
                int point_index_1 = old_index2new_index[triangle_points.at(0)];
                int point_index_2 = old_index2new_index[triangle_points.at(1)];
                int point_index_3 = old_index2new_index[triangle_points.at(2)];

                gp_Pnt point_1 = temp_points.at(point_index_1);
                gp_Pnt point_2 = temp_points.at(point_index_2);
                gp_Pnt point_3 = temp_points.at(point_index_3);

                TopoDS_Vertex vertex1 = BRepBuilderAPI_MakeVertex(point_1);
                TopoDS_Vertex vertex2 = BRepBuilderAPI_MakeVertex(point_2);
                TopoDS_Vertex vertex3 = BRepBuilderAPI_MakeVertex(point_3);

                TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(vertex1, vertex2);
                TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(vertex1, vertex3);
                TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(vertex2, vertex3);


                std::vector<int> points_egde_1{ point_index_1,point_index_2 };
                std::vector<int> points_egde_2{ point_index_1,point_index_3 };
                std::vector<int> points_egde_3{ point_index_2,point_index_3 };
                insert_edge_if_not_in(&edges_ones, &points_egde_1);
                insert_edge_if_not_in(&edges_ones, &points_egde_2);
                insert_edge_if_not_in(&edges_ones, &points_egde_3);

                TopoDS_Wire triangle_wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3);
                TopoDS_Face triangle_face = BRepBuilderAPI_MakeFace(triangle_wire);
                builder.Add(surface_top_shell, triangle_face);
            }
            
            //
            std::cout << "start vis" << std::endl;
            TopoDS_Wire bottom_wire_TEMP;
            builder.MakeWire(bottom_wire_TEMP);
            
            for (std::vector<int> one_bound_edge : edges_ones)
            {
                gp_Pnt point_1_bottom = temp_points.at(one_bound_edge.at(0));
                point_1_bottom.SetZ(z_min);
                TopoDS_Vertex vertex1_bottom = BRepBuilderAPI_MakeVertex(point_1_bottom);
                gp_Pnt point_2_bottom = temp_points.at(one_bound_edge.at(0));
                point_2_bottom.SetZ(z_min);
                TopoDS_Vertex vertex2_bottom = BRepBuilderAPI_MakeVertex(point_2_bottom);

                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(vertex2_bottom, vertex1_bottom);
                builder.Add(bottom_wire_TEMP, edge);
            }
            //TopoDS_Face bottom_TEMP = BRepBuilderAPI_MakeFace(bottom_wire_TEMP);
            visualize_figure(&bottom_wire_TEMP);
            //
            get_outer_counter(&edges_ones, &edges_external);
            for (std::vector<int> one_bound_edge : edges_external)
            {
                gp_Pnt point_1 = temp_points.at(one_bound_edge.at(0));
                gp_Pnt point_1_bottom = temp_points.at(one_bound_edge.at(0));
                point_1_bottom.SetZ(z_min);

                gp_Pnt point_2 = temp_points.at(one_bound_edge.at(1));
                gp_Pnt point_2_bottom = temp_points.at(one_bound_edge.at(1));
                point_2_bottom.SetZ(z_min);

                TopoDS_Vertex vertex1 = BRepBuilderAPI_MakeVertex(point_1);
                TopoDS_Vertex vertex1_bottom = BRepBuilderAPI_MakeVertex(point_1_bottom);
                TopoDS_Vertex vertex2 = BRepBuilderAPI_MakeVertex(point_2);
                TopoDS_Vertex vertex2_bottom = BRepBuilderAPI_MakeVertex(point_2_bottom);

                TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(vertex1, vertex2);
                TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(vertex2, vertex2_bottom);
                TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(vertex1_bottom, vertex1);
                TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(vertex2_bottom, vertex1_bottom);

                TopoDS_Wire triangle_wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3, edge4);
                TopoDS_Face triangle_face = BRepBuilderAPI_MakeFace(triangle_wire);

                builder.Add(surface_top_shell, triangle_face);
            }
            TopoDS_Wire bottom_wire;
            builder.MakeWire(bottom_wire);
            for (std::vector<int> one_bound_edge : edges_external)
            {
                gp_Pnt point_1_bottom = temp_points.at(one_bound_edge.at(0));
                point_1_bottom.SetZ(z_min);
                TopoDS_Vertex vertex1_bottom = BRepBuilderAPI_MakeVertex(point_1_bottom);
                gp_Pnt point_2_bottom = temp_points.at(one_bound_edge.at(0));
                point_2_bottom.SetZ(z_min);
                TopoDS_Vertex vertex2_bottom = BRepBuilderAPI_MakeVertex(point_2_bottom);

                TopoDS_Edge edge = BRepBuilderAPI_MakeEdge(vertex2_bottom, vertex1_bottom);
                builder.Add(bottom_wire, edge);
            }
            builder.Add(surface_top_shell, bottom_wire);
            //проверить замкнутая ли структура .... 
            TopoDS_Solid solid = BRepBuilderAPI_MakeSolid(surface_top_shell);

            //запись в IGES
            std::string surface_name_str = surface_name;
            std::string save_file_path = ReplaceAll(dir_path, ".xml", "_" + surface_name_str + ".xml");
            IGESControl_Controller::Init();
            IGESControl_Writer ICW(save_file_path.c_str(), 0);
            //creates a writer object for writing in Face mode with  millimeters 
            //TopoDS_Shape sh;
            ICW.AddShape(solid);
            //adds shape sh to IGES model 
            ICW.ComputeModel();
            Standard_Boolean OK = ICW.Write(save_file_path.c_str());
            //writes a model to the file MyFile.igs 
        }
    }
    //return 1;

}

