#include "external_functions.h"
#include <iostream>
#include <windows.h>
#include <filesystem>
#include <comdef.h>
#include <fstream>

#include <map>
#include <math.h>
#include <algorithm>
#include "aux_functions.h"
//#include <stdlib.h>
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

#include <BRepCheck_Status.hxx>
#include <ShapeFix_Solid.hxx>
#include <ShapeFix_Shell.hxx>

#include <IGESControl_Controller.hxx> 
#include <IGESControl_Writer.hxx> 
#include <TopoDS_Shape.hxx> 

#include <ShapeFix_Shape.hxx>
#include <ShapeFix_Wire.hxx>

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
void run_landxml2iges(const char* dir_path, double global_offset[])
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

                std::vector<double> current_point;
                std::stringstream ss(one_point.first_child().value());
                std::string item;

                while (getline(ss, item, ' ')) {
                    current_point.push_back(::atof(item.c_str()));
                }
                //Отнимаем от координат глобальный сдвиг
                std::cout << "new_point_PRE  " << current_point.at(0) << " " << current_point.at(1) << " " << current_point.at(2) << " " << std::endl;
                //y x z ::order of numbers in landxml definition
                current_point.at(0) -= global_offset[1];
                current_point.at(1) -= global_offset[0];
                current_point.at(2) -= global_offset[2];
                
                double new_x = current_point.at(1) * cos(global_offset[3]) - current_point.at(0) * sin(global_offset[3]);
                double new_y = current_point.at(1) * sin(global_offset[3]) + current_point.at(0) * cos(global_offset[3]);
                double new_z = current_point.at(2);
                new_x *= 1000.0;
                new_y *= 1000.0;
                new_z *= 1000.0;
                
                std::cout << "new_point_AFTER  " << new_x << " " << new_y << " " << new_z << " " << std::endl;

                //gp_Pnt new_point(current_point.at(0), current_point.at(1), current_point.at(2));
                gp_Pnt new_point(new_x, new_y, new_z);
                //std::cout << "new_point  " << new_point.X() << " " << new_point.Y() << " " << new_point.Z() << " " << std::endl;
                if (new_z < z_min) z_min = new_z;
                temp_points.push_back(new_point);
                //points_list[counter_points] = new_point;
                old_index2new_index.insert(std::pair<int, int>{old_point_index, counter_points});
                counter_points++;
            }
            z_min -= 500.0;

            /*
            Переходим к конструировавнию структуры оболочки солида.
            У нас будут 3 части - поверхность по верху surface_top_shell (= исходной поверхности);
            поверхность по бокам (3-угольные грани) и поверхность по дну в форме замкнутой кривой bottom_wire (ломаной);
            
            
            */
            BRep_Builder builder;
            TopoDS_Shell surface_top_shell;
            TopoDS_Shell surface_side_shell;
            TopoDS_Wire bottom_wire;
            builder.MakeWire(bottom_wire);
            builder.MakeShell(surface_top_shell);
            builder.MakeShell(surface_side_shell);

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
                edges_ones.push_back(points_egde_1);
                edges_ones.push_back(points_egde_2);
                edges_ones.push_back(points_egde_3);

                TopoDS_Wire triangle_wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3);
                TopoDS_Face triangle_face = BRepBuilderAPI_MakeFace(triangle_wire);
                builder.Add(surface_top_shell, triangle_face);
            }
            //Сортировка из набора ребер только тех, что повторяются единожды -- внешние ребра.
            //Внутренние если есть пока нет учета!!!!!
            sort_edges_to_unique(&edges_ones, &edges_external);

            //get_outer_counter(&edges_ones, &edges_external);

            int counter_new_tr = 0;
            for (std::vector<int> one_bound_edge : edges_external)
            {
                std::cout << "start for " << counter_new_tr << std::endl;
                std::cout << one_bound_edge.at(0) << " " << one_bound_edge.at(1) << std::endl;

                gp_Pnt point_1 = temp_points.at(one_bound_edge.at(0));
                
                gp_Pnt point_1_bottom = temp_points.at(one_bound_edge.at(0));
                point_1_bottom.SetZ(z_min);
                std::cout << "point-1 " << point_1.X() << " " << point_1.Y() << " " << point_1_bottom.Z() << " " << std::endl;

                gp_Pnt point_2 = temp_points.at(one_bound_edge.at(1));
                
                gp_Pnt point_2_bottom = temp_points.at(one_bound_edge.at(1));
                point_2_bottom.SetZ(z_min);
                std::cout << "point-2 " << point_2.X() << " " << point_2.Y() << " " << point_2_bottom.Z() << " " << std::endl;

                //Формимруем структуру для 4-угольной боковой грани
                TopoDS_Vertex vertex1 = BRepBuilderAPI_MakeVertex(point_1);
                TopoDS_Vertex vertex1_bottom = BRepBuilderAPI_MakeVertex(point_1_bottom);
                TopoDS_Vertex vertex2 = BRepBuilderAPI_MakeVertex(point_2);
                TopoDS_Vertex vertex2_bottom = BRepBuilderAPI_MakeVertex(point_2_bottom);

                TopoDS_Edge edge1 = BRepBuilderAPI_MakeEdge(vertex1, vertex2);
                TopoDS_Edge edge2 = BRepBuilderAPI_MakeEdge(vertex2, vertex2_bottom);
                TopoDS_Edge edge3 = BRepBuilderAPI_MakeEdge(vertex2_bottom, vertex1_bottom);
                TopoDS_Edge edge4 = BRepBuilderAPI_MakeEdge(vertex1_bottom, vertex1);
                
                //Добавляем в нижний контур сегмент - ребро
                TopoDS_Wire triangle_wire = BRepBuilderAPI_MakeWire(edge1, edge2, edge3, edge4);
                TopoDS_Face triangle_face = BRepBuilderAPI_MakeFace(triangle_wire);
                builder.Add(surface_top_shell, triangle_face);
                builder.Add(bottom_wire, edge3);
                counter_new_tr++;
                
            }
            std::cout << "end" << std::endl;
            //Оптимизация нижнего провода; mm
            TopoDS_Face bottom_face = BRepBuilderAPI_MakeFace(bottom_wire, false);
            Handle(ShapeFix_Wire) aFixWire_bottom = new ShapeFix_Wire(bottom_wire, bottom_face, 1.0);
            aFixWire_bottom->FixReorder();
            aFixWire_bottom->FixConnected();
            TopoDS_Wire aNewWire = aFixWire_bottom->Wire();
            TopoDS_Face bottom_face_new = BRepBuilderAPI_MakeFace(aNewWire, false);
            builder.Add(surface_top_shell, bottom_face_new);
            
            //visualize_figure(&pre_solid);
            double precision = 1;
            double toler = 0.5;
            // creation of tool and its initialization by shape
            Handle(ShapeFix_Shape) aFixShape = new ShapeFix_Shape(surface_top_shell);
            //aFixShape->Fix
            // set work precision and max allowed tolerance
            aFixShape->SetPrecision(precision);
            aFixShape->SetMaxTolerance(toler);
            //set the value of flag for forcing the removal of 2D curves
            //aFixShape->FixWireTool()->FixRemovePCurveMode() = 2;
            // reform fixes
            aFixShape->Perform();
            // getting the result
            if (aFixShape->Status(ShapeExtend_DONE))
            {
                
                std::cout << "Shape was fixed\n";
                TopoDS_Shape aResFace = aFixShape->Shape();
                std::cout << "Is closed" << aResFace.Closed() << "\n";

                TopoDS_Solid pre_solid;
                builder.MakeSolid(pre_solid);
                builder.Add(pre_solid, aResFace);

                //запись в IGES
                std::string surface_name_str = surface_name;
                std::string save_file_path = ReplaceAll(dir_path, ".xml", "_" + surface_name_str + ".iges");
                IGESControl_Controller::Init();
                IGESControl_Writer ICW(save_file_path.c_str(), 0);
                //creates a writer object for writing in Face mode with  millimeters 
                //TopoDS_Shape sh;
                ICW.AddShape(aResFace);
                //adds shape sh to IGES model 
                ICW.ComputeModel();
                bool check_status_write;
                Standard_Boolean OK = ICW.Write(save_file_path.c_str());
                //std::cout << "check_status_write " << check_status_write << std::endl;
                std::cout << "check_status_write OK " << OK << std::endl;

            }
            else if (aFixShape->Status(ShapeExtend_FAIL))
            {
                std::cout << "Shape could not be fixed\n";
            }
            else if (aFixShape->Status(ShapeExtend_OK))
            {
                std::cout << "Initial face is valid with specified precision =" << precision << std::endl;
            }
            ////проверить замкнутая ли структура .... 0 = false, 1 or each other = true 
            ////visualize_figure(&surface_top_shell);
            //ShapeFix_Shell my_shell(surface_top_shell);
            //my_shell.Perform();
            //TopoDS_Shell shell_new = my_shell.Shell();

            //auto is_shell_closed = shell_new.Closed();
            //std::cout << "surface_top_shell " << is_shell_closed << std::endl;
            //

            //
            //
            //ShapeFix_Solid my_solid(pre_solid);
            //const Message_ProgressRange mes;
            //bool shell_progress = my_solid.Perform(mes);
            ////
            //TopoDS_Shape solid = my_solid.Solid();
            ////visualize_figure(&solid);

            ////TopoDS_Solid solid = BRepBuilderAPI_MakeSolid(surface_top_shell);
            //auto is_closed = solid.Closed();
            //std::cout << "is_closed " << is_closed << std::endl;
            //
            //
            ////writes a model to the file MyFile.igs 
        }
    }
    //return 1;

}

