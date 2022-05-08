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