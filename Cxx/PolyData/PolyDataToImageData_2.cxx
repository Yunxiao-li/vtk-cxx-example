#include <vtkImageData.h>
#include <vtkImageStencil.h>
#include <vtkMetaImageWriter.h>
#include <vtkNew.h>
#include <vtkSmartPointer.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataToImageStencil.h>
#include <vtkSphereSource.h>
#include <vtkSTLReader.h>
#include <vtkPLYReader.h>
#include <vtkGenericDataObjectReader.h>
#include <vtkXMLImageDataWriter.h>
#include <vtkStructuredPointsReader.h>
#include <vtkUnstructuredGridReader.h>
#include <vtkUnstructuredGrid.h>
#include <vtkDataObject.h>
#include <vtkDataSetAttributes.h>
#include <vtkPolyDataReader.h>
#include <iostream>
#include <string>
/**
 * This program generates a sphere (closed surface, vtkPolyData) and converts it
 * into volume representation (vtkImageData) where the foreground voxels are 1
 * and the background voxels are 0. Internally vtkPolyDataToImageStencil is
 * utilized. The resultant image is saved to disk in metaimage file format
 * (SphereVolume.mhd).
 */
void GetPolydataBounds(vtkSmartPointer<vtkPolyData> polydata, double bounds[6]) {
    vtkIdType size = polydata->GetNumberOfPoints();
    vtkPoints *points = polydata->GetPoints();
    double *point = points->GetPoint(0);
    bounds[0] = point[0];
    bounds[1] = point[0];
    bounds[2] = point[1];
    bounds[3] = point[2];
    bounds[4] = point[3];
    bounds[5] = point[4];
    for (int i=1; i<size; ++i) {
      point = points->GetPoint(i);
      if (point[0] < bounds[0]) {
        bounds[0] = point[0];
      }
      if (point[1] < bounds[2]) {
        bounds[2] = point[1];
      }
      if (point[2] < bounds[4]) {
        bounds[4] = point[2];
      }
      if (point[0] > bounds[1]) {
        bounds[1] = point[0];
      }
      if (point[1] > bounds[3]) {
        bounds[3] = point[1];
      }
      if (point[2] > bounds[5]) {
        bounds[5] = point[2];
      }
    }
    std::cout << "bounds: " << bounds[0] << ", " << bounds[1] << ", "<< bounds[2] 
    << ", "<< bounds[3] << ", "<< bounds[4] << ", "<< bounds[5] << std::endl;
}

vtkSmartPointer<vtkPolyData> ReadMeshFile(const std::string &filename){
  if (filename.find(".stl") != std::string::npos) {
    vtkNew<vtkSTLReader> reader;
    reader->SetFileName(filename.c_str());
    return reader->GetOutput();
  } else if (filename.find(".ply") != std::string::npos) {
    vtkNew<vtkPLYReader>  reader;
    reader->SetFileName(filename.c_str());
    double bounds[6];
    reader->GetOutput()->GetBounds(bounds);
    return reader->GetOutput();
  } else if (filename.find(".vtk") != std::string::npos) {
    auto MeshReader = vtkSmartPointer<vtkGenericDataObjectReader>::New();
    MeshReader->SetFileName(filename.c_str());
    MeshReader->Update();
    return MeshReader->GetPolyDataOutput();
    //----------- for test -------------
    if (1) {
      vtkNew<vtkPolyDataReader> reader;
      reader->SetFileName(filename.c_str());
      double bounds[6] = {0,0,0,0,0,0};
      reader->GetOutput()->GetBounds(bounds);
      return reader->GetOutput();
    } else {
      vtkNew<vtkStructuredPointsReader> reader;
      reader->ReadAllVectorsOn();
      reader->ReadAllScalarsOn();
      reader->SetFileName(filename.c_str());
    }      
  } else {
    std::cout << "unknown file format" << std::endl;
    vtkSmartPointer<vtkPolyData> temp;
    return temp;
  }
}

vtkSmartPointer<vtkImageData> Polydata2ImageData(vtkSmartPointer<vtkPolyData> pd, double *spacing, int padding=1) {
  vtkNew<vtkImageData> whiteImage;
  double bounds[6];
  pd->GetBounds(bounds);
  if (abs(bounds[1] - bounds[0]) < 8 || abs(bounds[3] - bounds[2]) ) {
    std::cout << "getbounds from custom function \n";
    GetPolydataBounds(pd, bounds);
  }
  
  // compute dimensions
  int dim[3];
  for (int i = 0; i < 3; i++)
  {
    dim[i] = static_cast<int>(
        ceil((bounds[i * 2 + 1] - bounds[i * 2]) / spacing[i]));
  }

  double origin[3];
  origin[0] = bounds[0] + spacing[0] / 2;
  origin[1] = bounds[2] + spacing[1] / 2;
  origin[2] = bounds[4] + spacing[2] / 2;
  if (padding) {
    origin[0] -= spacing[0];
    origin[1] -= spacing[1];
    origin[2] -= spacing[2];

    dim[0] += 2 * padding;
    dim[1] += 2 * padding;
    dim[2] += 2 * padding;
  }
  whiteImage->SetSpacing(spacing);
  whiteImage->SetDimensions(dim);
  whiteImage->SetExtent(0, dim[0] - 1, 0, dim[1] - 1, 0, dim[2] - 1);
  whiteImage->SetOrigin(origin);
  whiteImage->AllocateScalars(VTK_UNSIGNED_CHAR, 1);

  // fill the image with foreground voxels:
  unsigned char inval = 255;
  unsigned char outval = 0;
  vtkIdType count = whiteImage->GetNumberOfPoints();
  for (vtkIdType i = 0; i < count; ++i)
  {
    whiteImage->GetPointData()->GetScalars()->SetTuple1(i, inval);
  }

  // polygonal data --> image stencil:
  vtkNew<vtkPolyDataToImageStencil> pol2stenc;
  pol2stenc->SetInputData(pd);
  pol2stenc->SetOutputOrigin(origin);
  pol2stenc->SetOutputSpacing(spacing);
  pol2stenc->SetOutputWholeExtent(whiteImage->GetExtent());
  pol2stenc->Update();

  // cut the corresponding white image and set the background:
  vtkNew<vtkImageStencil> imgstenc;
  imgstenc->SetInputData(whiteImage);
  imgstenc->SetStencilConnection(pol2stenc->GetOutputPort());
  imgstenc->ReverseStencilOff();
  imgstenc->SetBackgroundValue(outval);
  imgstenc->Update();
  return imgstenc->GetOutput();
}

void SaveImagedata2VTI(vtkSmartPointer<vtkImageData> imagedata, const std::string &filename) {
  vtkNew<vtkXMLImageDataWriter> writer;
  double *bounds = imagedata->GetBounds();
  double *origin = imagedata->GetOrigin();
  std::cout << "out bounds: " << bounds[0] << ", " << bounds[1] << ", "<< bounds[2] 
    << ", "<< bounds[3] << ", "<< bounds[4] << ", "<< bounds[5] << std::endl;
  std::cout << "out origin: " << origin[0] << ", " << origin[1] << ", "<< origin[2] << std::endl;
  
  writer->SetFileName(filename.c_str());
  writer->SetInputData(imagedata);
  writer->Write();
}

int main(int argc, char*[])
{
  vtkSmartPointer<vtkPolyData> polydata = ReadMeshFile("mesh.vtk");
  if (polydata.Get() == nullptr) {
    std::cout << "polydata in null \n";
    return 1;
  }
  double spacing[3] = {10, 10, 10};
  vtkSmartPointer<vtkImageData> imagedata = Polydata2ImageData(polydata, spacing);
  SaveImagedata2VTI(imagedata, "mesh_c.vti");

  return 0;
}
