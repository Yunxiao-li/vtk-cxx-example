#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkClipClosedSurface.h>
#include <vtkDataSetMapper.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkPlane.h>
#include <vtkPlaneCollection.h>
#include <vtkPolyData.h>
#include <vtkProperty.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>
#include <vtkXMLPolyDataReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkFillHolesFilter.h>
#include <vtkPolyDataMapper.h>

//
// Demonstrate the use of clipping of polygonal data
//

int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;

  // PolyData to process
  vtkSmartPointer<vtkPolyData> polyData;

  if (argc > 1)
  {
    // vtk file reader
    vtkNew<vtkGenericDataObjectReader> reader;
    reader->SetFileName(argv[1]);
    reader->Update();
    // vtkNew<vtkGenericDataObjectReader> reader;
    // vtkNew<vtkXMLPolyDataReader> reader;
    // reader->SetFileName(argv[1]);
    // reader->Update();
    polyData = reader->GetPolyDataOutput();
  }
  else
  {
    // Create a sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetThetaResolution(20);
    sphereSource->SetPhiResolution(11);
    sphereSource->Update();

    polyData = sphereSource->GetOutput();
  }

  vtkNew<vtkPlaneCollection> planes;
  if (1) { // test
    // double origins[6][3] = {
    //   {-138.083, -2.12001, -599.835},
    //   { 121.6, -2.12001, -599.835},
    //   { -8.24152, -334.14, -599.835},
    //   { -8.24152, 329.9, -599.835},
    //   { -8.24152, -2.12001, -1581.1},
    //   { -8.24152, -2.12001, 381.43} };
    double origins[6][3] = {
      {-284.24, 101.535, -595.649},
      { 97.946, 101.535, -595.649},
      { -93.147, -160.48, -595.649},
      { -93.147, 363.55, -595.649},
      { -93.147, 101.535, -987.029},
      { -93.147, 101.535, -204.27} };
    // double normals[6][3] = {
    //   {-1, 0, 0},
    //   { 1, 0, 0},
    //   { 0, -1, -0},
    //   { 0, 1, 0},
    //   { 0, 0, -1},
    //   { 0, 0, 1}};
    // boxwidget normal is inverse
      double normals[6][3] = {
      {1, 0, 0},
      { -1, 0, 0},
      { 0, 1, -0},
      { 0, -1, 0},
      { 0, 0, 1},
      { 0, 0, -1}};

      for (int i=0; i<6; ++i) {
        vtkNew<vtkPlane> plane;
        plane->SetOrigin(origins[i]);
        plane->SetNormal(normals[i]);
        planes->AddItem(plane);
      }
  } else {
    auto center = polyData->GetCenter();
    vtkNew<vtkPlane> plane1;
    plane1->SetOrigin(center[0], center[1], center[2]);
    plane1->SetNormal(0.0, -1.0, 0.0);
    vtkNew<vtkPlane> plane2;
    plane2->SetOrigin(center[0], center[1], center[2]);
    plane2->SetNormal(0.0, 0.0, 1.0);
    vtkNew<vtkPlane> plane3;
    plane3->SetOrigin(center[0], center[1], center[2]);
    plane3->SetNormal(-1.0, 0.0, 0.0);
    planes->AddItem(plane1);
    // planes->AddItem(plane2);
    // planes->AddItem(plane3);
  }

  vtkNew<vtkClipClosedSurface> clipper;
  clipper->SetInputData(polyData);
  clipper->SetClippingPlanes(planes);
  // clipper->SetActivePlaneId(2);
  clipper->SetScalarModeToColors();
  clipper->SetClipColor(colors->GetColor3d("Banana").GetData());
  clipper->SetBaseColor(colors->GetColor3d("Tomato").GetData());
  clipper->SetActivePlaneColor(colors->GetColor3d("SandyBrown").GetData());
  
# if 0
  vtkNew<vtkDataSetMapper> clipMapper;
  clipMapper->SetInputConnection(clipper->GetOutputPort());
# else
  vtkNew<vtkPolyDataMapper> clipMapper;  
  vtkNew<vtkFillHolesFilter> fillHolesFilter;
  fillHolesFilter->SetInputConnection(clipper->GetOutputPort());
  // this not working, vtkClipPolyData workable see BoxWidget2Clipping.cxx
  // fillHolesFilter->SetInputData(clipper->GetOutput());
  fillHolesFilter->SetHoleSize(100000.0);
  fillHolesFilter->Update();
  clipMapper->SetInputData(fillHolesFilter->GetOutput());
#endif

  vtkNew<vtkActor> clipActor;
  clipActor->SetMapper(clipMapper);
  clipActor->GetProperty()->SetColor(1.0000, 0.3882, 0.2784);
  clipActor->GetProperty()->SetInterpolationToFlat();

  // Create graphics stuff
  //
  vtkNew<vtkRenderer> ren1;
  ren1->SetBackground(colors->GetColor3d("SteelBlue").GetData());

  vtkNew<vtkRenderWindow> renWin;
  renWin->AddRenderer(ren1);
  renWin->SetSize(512, 512);
  renWin->SetWindowName("ClipClosedSurface");

  vtkNew<vtkRenderWindowInteractor> iren;
  iren->SetRenderWindow(renWin);

  // Add the actors to the renderer, set the background and size
  //
  ren1->AddActor(clipActor);

  // Generate an interesting view
  //
  ren1->ResetCamera();
  // ren1->GetActiveCamera()->Azimuth(120);
  // ren1->GetActiveCamera()->Elevation(30);
  // ren1->GetActiveCamera()->Dolly(1.0);
  // ren1->ResetCameraClippingRange();

  renWin->Render();
  iren->Initialize();
  iren->Start();

  return EXIT_SUCCESS;
}
