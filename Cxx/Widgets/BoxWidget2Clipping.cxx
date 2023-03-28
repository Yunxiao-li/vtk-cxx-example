#include <vtkSmartPointer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkBoxRepresentation.h>
#include <vtkDataSetMapper.h>
#include <vtkCallbackCommand.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkProperty.h>
#include <vtkPlane.h>
#include <vtkBox.h>
#include <vtkClipPolyData.h>
#include <vtkPlanes.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkSphereSource.h>
#include <vtkBoxWidget2.h>
#include "vtkGenericDataObjectReader.h"
#include "vtkPolyDataMapper.h"
#include "vtkDataArray.h"
#include "vtkDoubleArray.h"
#include "vtkAlgorithmOutput.h"
#include <vtkSTLWriter.h>
#include "vtkFillHolesFilter.h"
#include "vtkPolyDataMapper.h"

#include <tuple>

/// @brief using vtkClipPolyData to clip polydata
/// using vtkFillHolesFilter to generate closet polydata
/// using vtkBoxWidget2 to generate clip planes
int main(int argc, char *argv[])
{
  //box widget
  vtkSmartPointer<vtkBoxWidget2> boxWidget = vtkSmartPointer<vtkBoxWidget2>::New();
  vtkSmartPointer<vtkClipPolyData> clipperPoly = vtkSmartPointer<vtkClipPolyData>::New();
  // vtkNew<vtkClipPolyData> clip;
  // vtkNew<vtkPolyData> polydata;
  if (argc > 1) {
    // vtk file
    auto MeshReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    MeshReader->SetFileName(argv[1]);
    MeshReader->Update();
    // storing planes of the bounding box
    vtkSmartPointer<vtkPlanes> planesClipping = vtkSmartPointer<vtkPlanes>::New();
    vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetPlanes(planesClipping);
    
    // for test: originlly clip the input data
    if (1) {
      // clip origin data
      vtkNew<vtkPlanes> planes;
      vtkNew<vtkPoints> points;
      points->SetDataTypeToDouble();
      points->SetNumberOfPoints(6);
      points->InsertPoint(0, -138.083, -2.12001, -599.835);
      points->InsertPoint(1, 121.6, -2.12001, -599.835);
      points->InsertPoint(2, -8.24152, -334.14, -599.835);
      points->InsertPoint(3, -8.24152, 329.9, -599.835);
      points->InsertPoint(4, -8.24152, -2.12001, -1581.1);
      points->InsertPoint(5, -8.24152, -2.12001, 381.43);
      // points->InsertNextPoint(-138.083, -2.12001, -599.835); // not working
      // points->InsertNextPoint(121.6, -2.12001, -599.835);
      // points->InsertNextPoint(-8.24152, -334.14, -599.835);
      // points->InsertNextPoint(-8.24152, 329.9, -599.835);
      // points->InsertNextPoint(-8.24152, -2.12001, -1581.1);
      // points->InsertNextPoint(-8.24152, -2.12001, 381.43);
      vtkNew<vtkDoubleArray> normalarr;
      normalarr->SetNumberOfComponents(3);
      normalarr->SetNumberOfTuples(6);
      double normal0[3] = {-1, 0, 0};
      double normal1[3] = {1, 0, 0};
      double normal2[3] = {0, -1, -0};
      double normal3[3] = {0, 1, 0};
      double normal4[3] = {0, 0, -1};
      double normal5[3] = {0, 0, 1};
      normalarr->InsertTuple(0, normal0); 
      normalarr->InsertTuple(1, normal1); 
      normalarr->InsertTuple(2, normal2); 
      normalarr->InsertTuple(3, normal3); 
      normalarr->InsertTuple(4, normal4); 
      normalarr->InsertTuple(5, normal5);    
      // normalarr->InsertNextTuple(normal0);
      // normalarr->InsertNextTuple(normal1);
      // normalarr->InsertNextTuple(normal2);
      // normalarr->InsertNextTuple(normal3);
      // normalarr->InsertNextTuple(normal4);
      // normalarr->InsertNextTuple(normal5);
      
      planes->SetPoints(points);
      planes->SetNormals(normalarr);
      // vtkPoints* tempp = planes->GetPoints();
      // tempp->PrintSelf(std::cout, vtkIndent(0));
      double origin[3];
      double normal[3];
      int planesize = planesClipping->GetNumberOfPlanes();
      for (int i=0; i<planesize; ++i) {
        planesClipping->GetPlane(i)->GetOrigin(origin);
        planesClipping->GetPlane(i)->GetNormal(normal);
        // std::cout << "origin " << i << ": " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
        // std::cout << "normal " << i << ": " << normal[0] << ", " << normal[1] << ", " << normal[2] << std::endl;
      }
      vtkNew<vtkClipPolyData> clip;
      clip->SetInputData(MeshReader->GetOutput());
      clip->SetClipFunction(planes);
      clip->InsideOutOn();
      clip->GenerateClipScalarsOn();
      clip->Update();
      
      // clip->GetOutputPort()->PrintSelf(std::cout, vtkIndent(0));
      // clip->GetOutput()->PrintSelf(std::cout, vtkIndent(0));
      // vtkNew<vtkPolyData> polydata;
      // polydata->DeepCopy(clip->GetClippedOutput());
      // polydata->PrintSelf(std::cout, vtkIndent(0));
      #if 1 // test fillholesfilter
      vtkNew<vtkFillHolesFilter> fillHolesFilter;
      // fillHolesFilter->SetInputConnection(clip->GetOutputPort());
      fillHolesFilter->SetInputData(clip->GetOutput());
      fillHolesFilter->SetHoleSize(100000.0);
      fillHolesFilter->Update();
      if (0) {
        // write closed surface to file
        vtkNew<vtkSTLWriter> writer;
        writer->SetFileName("nfz_closed.stl");
        writer->SetInputConnection(fillHolesFilter->GetOutputPort());
        writer->Write();
        std::cout << "save closed surface stl success \n";
      }

      clipperPoly->SetInputData(fillHolesFilter->GetOutput());
      clipperPoly->SetClipFunction(planesClipping);
      #else
      clipperPoly->SetInputData(clip->GetOutput());
      clipperPoly->SetClipFunction(planesClipping);
      #endif
      // write polydata to stl file
      if (0) {
        vtkNew<vtkSTLWriter> writer;
        writer->SetFileName("noflyzone.stl");
        writer->SetInputConnection(clip->GetOutputPort());
        writer->Write();
      }
    } else {
      // clipping structure    
      clipperPoly->SetInputData(MeshReader->GetPolyDataOutput());
      clipperPoly->SetClipFunction(planesClipping);
    }
  // using vtk sphere  
  } else {
    // Create a sphere
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetThetaResolution(32);
    sphereSource->SetPhiResolution(32);
    sphereSource->Update();

    // storing planes of the bounding box
    vtkSmartPointer<vtkPlanes> planesClipping = vtkSmartPointer<vtkPlanes>::New();
    vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetPlanes(planesClipping);

    // clipping structure    
    // clipperPoly->SetInputConnection(sphereSource->GetOutputPort()); //also clipperPoly->SetInputData(sphereSource->GetOutput());
    clipperPoly->SetInputData(sphereSource->GetOutput()); // OK
    clipperPoly->SetClipFunction(planesClipping);
  }
  
  //mapper
  vtkSmartPointer<vtkDataSetMapper> selectMapper = vtkSmartPointer<vtkDataSetMapper>::New();
  selectMapper->SetInputConnection(clipperPoly->GetOutputPort());
  // selectMapper->SetInputData(clipperPoly->GetOutput()); // NOT WORKING
  // selectMapper->SetInputConnection(clip->GetOutputPort());
  // clip->GetOutput()->PrintSelf(std::cout, vtkIndent(1));
  // selectMapper->SetInputData(clip->GetOutput()); // workable
  selectMapper->Update();

  //actor
  vtkSmartPointer<vtkActor> selectActor = vtkSmartPointer<vtkActor>::New();
  selectActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);
  selectActor->SetMapper(selectMapper);

  // Create graphics stuff
  vtkSmartPointer<vtkRenderer> ren1 = vtkSmartPointer<vtkRenderer>::New();
  ren1->SetBackground(.2, .2, .25);

  vtkSmartPointer<vtkRenderWindow> renWin = vtkSmartPointer<vtkRenderWindow>::New();
  renWin->AddRenderer(ren1);
  renWin->SetSize(512, 512);

  // window interactor
  vtkSmartPointer<vtkRenderWindowInteractor> windowInteractor =  vtkSmartPointer<vtkRenderWindowInteractor>::New();
  windowInteractor->SetRenderWindow(renWin);

  //the box widget
  boxWidget->SetInteractor(windowInteractor);
  boxWidget->GetRepresentation()->SetPlaceFactor(1); // Default is 0.5
  boxWidget->GetRepresentation()->PlaceWidget(selectActor->GetBounds());
  boxWidget->On();

  //callback function
  auto interactionFcn = [](vtkObject* caller, long unsigned int eventId, void* clientData, void* callData) {
    vtkSmartPointer<vtkBoxWidget2> boxWidget = vtkBoxWidget2::SafeDownCast(caller);
    vtkSmartPointer<vtkPlanes> planesClipping = vtkSmartPointer<vtkPlanes>::New();
    vtkBoxRepresentation::SafeDownCast(boxWidget->GetRepresentation())->GetPlanes(planesClipping);

    //get the planes
    vtkClipPolyData* clipperPoly = static_cast<vtkClipPolyData*>(clientData);
    clipperPoly->SetClipFunction(planesClipping);
    clipperPoly->InsideOutOn();
    // ------- for test ----------
    vtkDataArray *arr = planesClipping->GetNormals();
    vtkIdType arrsize = arr->GetDataSize();
    vtkPoints * points = planesClipping->GetPoints();
    double origin[3];
    double normal[3];
    int planesize = planesClipping->GetNumberOfPlanes();
    for (int i=0; i<planesize; ++i) {
      planesClipping->GetPlane(i)->GetOrigin(origin);
      planesClipping->GetPlane(i)->GetNormal(normal);
      std::cout << "origin " << i << ": " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
      std::cout << "normal " << i << ": " << normal[0] << ", " << normal[1] << ", " << normal[2] << std::endl;
    }
    vtkIndent indent(0);
    // points->PrintSelf(std::cout, vtkIndent(1));
    // clipperPoly = nullptr;
  };
  vtkSmartPointer<vtkCallbackCommand> interactionCallback = vtkSmartPointer<vtkCallbackCommand>::New();
  interactionCallback->SetCallback(interactionFcn);
  interactionCallback->SetClientData(clipperPoly);  //pass the clipper to update it
  boxWidget->AddObserver(vtkCommand::EndInteractionEvent, interactionCallback);

  // Add the actor
  ren1->AddActor(selectActor);


  // Generate a camera
  vtkSmartPointer<vtkInteractorStyleTrackballCamera> cameraStyle = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();
  windowInteractor->SetInteractorStyle(cameraStyle);
  ren1->ResetCamera();

  windowInteractor->Initialize();
  windowInteractor->Start();

  return EXIT_SUCCESS;
}