#include <vtkActor.h>
#include <vtkBoxRepresentation.h>
#include <vtkBoxWidget2.h>
#include <vtkCommand.h>
#include <vtkConeSource.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkTransform.h>
#include <vtkPlanes.h>
#include "vtkGenericDataObjectReader.h"

namespace {
class vtkBoxCallback : public vtkCommand
{
public:
  static vtkBoxCallback* New()
  {
    return new vtkBoxCallback;
  }

  vtkSmartPointer<vtkActor> m_actor;
  vtkSmartPointer<vtkPlanes> m_planes;

  void SetActor(vtkSmartPointer<vtkActor> actor)
  {
    m_actor = actor;
  }

  virtual void Execute(vtkObject* caller, unsigned long, void*)
  {
    vtkSmartPointer<vtkBoxWidget2> boxWidget =
        dynamic_cast<vtkBoxWidget2*>(caller);

    vtkNew<vtkTransform> t;
    
    auto *boxRep = vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation());
    dynamic_cast<vtkBoxRepresentation*>(boxWidget->GetRepresentation())
        ->GetTransform(t);
    // this->m_actor->SetUserTransform(t);
    boxRep->SetInsideOut(1);
    boxRep->GetPlanes(m_planes);
    this->m_actor->GetMapper()->SetClippingPlanes(m_planes);
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->ShallowCopy(this->m_actor->GetMapper());
    this->m_actor->SetMapper(mapper);
  }

  vtkBoxCallback()
  {
  }
};
} // namespace

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkPolyDataMapper> mapper;
  if (argc > 1) {
    // vtk file
    auto MeshReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    MeshReader->SetFileName(argv[1]);
    MeshReader->Update();
    mapper->SetInputData(MeshReader->GetPolyDataOutput());
  } else {
    vtkNew<vtkConeSource> coneSource;
    coneSource->SetHeight(1.5);
  
    mapper->SetInputConnection(coneSource->GetOutputPort());
  }
  

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetColor(colors->GetColor3d("BurlyWood").GetData());

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(colors->GetColor3d("Blue").GetData());
  renderer->ResetCamera(); // Reposition camera so the whole scene is visible

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);
  renderWindow->SetWindowName("BoxWidget2");

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  // Use the "trackball camera" interactor style, rather than the default
  // "joystick camera"
  vtkNew<vtkInteractorStyleTrackballCamera> style;
  renderWindowInteractor->SetInteractorStyle(style);

  vtkNew<vtkBoxWidget2> boxWidget;
  boxWidget->SetInteractor(renderWindowInteractor);
  boxWidget->GetRepresentation()->SetPlaceFactor(1); // Default is 0.5
  boxWidget->GetRepresentation()->PlaceWidget(actor->GetBounds());

  // Set up a callback for the interactor to call so we can manipulate the actor
  vtkNew<vtkBoxCallback> boxCallback;
  boxCallback->SetActor(actor);
  boxWidget->AddObserver(vtkCommand::InteractionEvent, boxCallback);

  boxWidget->On();

  renderWindow->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
