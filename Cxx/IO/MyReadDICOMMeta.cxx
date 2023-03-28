//
// This example demonstrates how to read a series of dicom images
// and how to scroll with the mousewheel or the up/down keys
// through all slices
//
// some standard vtk headers
#include <vtkActor.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
// headers needed for this example
#include <vtkActor2D.h>
#include <vtkDICOMImageReader.h>
#include <vtkImageViewer2.h>
#include <vtkInteractorStyleImage.h>
#include <vtkTextMapper.h>
#include <vtkTextProperty.h>
#include <vtkMetaImageReader.h>
#include <vtkAssemblyPath.h>
#include <vtkPropPicker.h>
#include <vtkImageActor.h>
#include <vtkImageData.h>
#include <vtkMatrix3x3.h>

// needed to easily convert int to std::string
#include <sstream>
#include <iostream>

namespace {

// helper class to format slice status message
class StatusMessage
{
public:
  static std::string Format(int slice, int maxSlice)
  {
    std::stringstream tmp;
    tmp << "Slice Number  " << slice + 1 << "/" << maxSlice + 1;
    return tmp.str();
  }
};

// Define own interaction style
class myVtkInteractorStyleImage : public vtkInteractorStyleImage
{
public:
  static myVtkInteractorStyleImage* New();
  vtkTypeMacro(myVtkInteractorStyleImage, vtkInteractorStyleImage);

protected:
  vtkImageViewer2* _ImageViewer;
  vtkTextMapper* _StatusMapper;
    // Pointer to the picker
  vtkPropPicker* _Picker;
  int _Slice;
  int _MinSlice;
  int _MaxSlice;

public:
  void SetImageViewer(vtkImageViewer2* imageViewer)
  {
    _ImageViewer = imageViewer;
    _MinSlice = imageViewer->GetSliceMin();
    _MaxSlice = imageViewer->GetSliceMax();
    _Slice = _MinSlice;
    cout << "Slicer: Min = " << _MinSlice << ", Max = " << _MaxSlice
         << std::endl;
  }

  void SetStatusMapper(vtkTextMapper* statusMapper)
  {
    _StatusMapper = statusMapper;
  }

  void SetPicker(vtkPropPicker* picker)
  {
    _Picker = picker;
  }

protected:
  void MoveSliceForward()
  {
    if (_Slice < _MaxSlice)
    {
      _Slice += 1;
      cout << "MoveSliceForward::Slice = " << _Slice << std::endl;
      _ImageViewer->SetSlice(_Slice);
      std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
      _StatusMapper->SetInput(msg.c_str());
      _ImageViewer->Render();
    }
  }

  void MoveSliceBackward()
  {
    if (_Slice > _MinSlice)
    {
      _Slice -= 1;
      cout << "MoveSliceBackward::Slice = " << _Slice << std::endl;
      _ImageViewer->SetSlice(_Slice);
      std::string msg = StatusMessage::Format(_Slice, _MaxSlice);
      _StatusMapper->SetInput(msg.c_str());
      _ImageViewer->Render();
    }
  }

  virtual void OnKeyDown()
  {
    std::string key = this->GetInteractor()->GetKeySym();
    if (key.compare("Up") == 0)
    {
      // cout << "Up arrow key was pressed." << endl;
      MoveSliceForward();
    }
    else if (key.compare("Down") == 0)
    {
      // cout << "Down arrow key was pressed." << endl;
      MoveSliceBackward();
    }
    // forward event
    vtkInteractorStyleImage::OnKeyDown();
  }

  virtual void OnLeftButtonDown()
  {                                      
    // Pick at the mouse location provided by the interactor
    this->_Picker->Pick(Interactor->GetEventPosition()[0],
                       Interactor->GetEventPosition()[1], 0.0, this->CurrentRenderer);

    // There could be other props assigned to this picker, so
    // make sure we picked the image actor
    vtkAssemblyPath* path = this->_Picker->GetPath();
    bool validPick = false;
    vtkImageActor* actor = this->_ImageViewer->GetImageActor();

    if (path)
    {
      vtkCollectionSimpleIterator sit;
      path->InitTraversal(sit);
      // vtkAssemblyNode *node;
      for (int i = 0; i < path->GetNumberOfItems() && !validPick; ++i)
      {
        auto node = path->GetNextNode(sit);
        if (actor == dynamic_cast<vtkImageActor*>(node->GetViewProp()))
        {
          validPick = true;
        }
        
      }
    }
    if (validPick)
    {
      // Get the world coordinates of the pick
      double picked[3];
      this->_Picker->GetPickPosition(picked);
      std::cout << "pick pos: " << picked[0] << ", " << picked[1] << ", " 
        << picked[2] << std::endl;
    }
    
    // Forward events
    vtkInteractorStyleImage::OnLeftButtonDown();

    // this->Interactor->GetRenderWindow()->Render();
    this->Interactor->Render();
  }

  virtual void OnMouseWheelForward()
  {
    // std::cout << "Scrolled mouse wheel forward." << std::endl;
    MoveSliceForward();
    // don't forward events, otherwise the image will be zoomed
    // in case another interactorstyle is used (e.g. trackballstyle, ...)
    // vtkInteractorStyleImage::OnMouseWheelForward();
  }

  virtual void OnMouseWheelBackward()
  {
    // std::cout << "Scrolled mouse wheel backward." << std::endl;
    if (_Slice > _MinSlice)
    {
      MoveSliceBackward();
    }
    // don't forward events, otherwise the image will be zoomed
    // in case another interactorstyle is used (e.g. trackballstyle, ...)
    // vtkInteractorStyleImage::OnMouseWheelBackward();
  }
};

vtkStandardNewMacro(myVtkInteractorStyleImage);

} // namespace

/// @brief read metadata and using vtkImageViewer2 to show slice
/// maily used to test the metadata transfomed by polydata, like polydata screw
/// @param argc 
/// @param argv 
/// @return 
int main(int argc, char* argv[])
{
  vtkNew<vtkNamedColors> colors;

  // Verify input arguments
  if (argc != 2)
  {
    std::cout << "Usage: " << argv[0] << " FolderName" << std::endl;
    return EXIT_FAILURE;
  }

  std::string folder = argv[1];
  // std::string folder = "C:\\VTK\\vtkdata-5.8.0\\Data\\DicomTestImages";

  // Read all the DICOM files in the specified directory.
  //vtkNew<vtkDICOMImageReader> reader;
  vtkNew<vtkMetaImageReader> reader;
  reader->SetFileName(folder.c_str());
  reader->Update();
  
  vtkImageData* image = reader->GetOutput();
  double origin[3], bounds[6];
  int extent[6]; 
  image->GetOrigin(origin);
  image->GetExtent(extent);
  image->GetBounds(bounds);
   
  std::cout << "origin: " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
  std::cout << "extent: " << extent[0] << ", " << extent[1] << ", " << extent[2] 
    << ", " << extent[3] << ", " << extent[4] << ", " << extent[5] << std::endl;
  std::cout << "bounds: " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] 
    << ", " << bounds[3] << ", " << bounds[4] << ", " << bounds[5] << std::endl; 
  vtkMatrix3x3 *mat = image->GetDirectionMatrix(); 
  mat->PrintSelf(std::cout, vtkIndent(1));
#if 1
  // new origin
  double neworigin[3] = {(bounds[0]+bounds[1])/2.0, origin[1], (bounds[4]+bounds[5])/2.0};
  image->SetOrigin(neworigin);
  
  std::cout << "--------------new-----------------\n";
  std::cout << "origin: " << origin[0] << ", " << origin[1] << ", " << origin[2] << std::endl;
  std::cout << "extent: " << extent[0] << ", " << extent[1] << ", " << extent[2] 
    << ", " << extent[3] << ", " << extent[4] << ", " << extent[5] << std::endl;
  std::cout << "bounds: " << bounds[0] << ", " << bounds[1] << ", " << bounds[2] 
    << ", " << bounds[3] << ", " << bounds[4] << ", " << bounds[5] << std::endl; 
  mat = image->GetDirectionMatrix(); 
  mat->PrintSelf(std::cout, vtkIndent(1));
#endif
  // Visualize
  vtkNew<vtkImageViewer2> imageViewer;
  imageViewer->SetInputConnection(reader->GetOutputPort());

  // Picker to pick pixels
  vtkNew<vtkPropPicker> propPicker;
  propPicker->PickFromListOn();
  propPicker->AddPickList(imageViewer->GetImageActor());

  // slice status message
  vtkNew<vtkTextProperty> sliceTextProp;
  sliceTextProp->SetFontFamilyToCourier();
  sliceTextProp->SetFontSize(20);
  sliceTextProp->SetVerticalJustificationToBottom();
  sliceTextProp->SetJustificationToLeft();

  vtkNew<vtkTextMapper> sliceTextMapper;
  std::string msg = StatusMessage::Format(imageViewer->GetSliceMin(),
                                          imageViewer->GetSliceMax());
  sliceTextMapper->SetInput(msg.c_str());
  sliceTextMapper->SetTextProperty(sliceTextProp);

  vtkNew<vtkActor2D> sliceTextActor;
  sliceTextActor->SetMapper(sliceTextMapper);
  sliceTextActor->SetPosition(15, 10);

  // usage hint message
  vtkNew<vtkTextProperty> usageTextProp;
  usageTextProp->SetFontFamilyToCourier();
  usageTextProp->SetFontSize(14);
  usageTextProp->SetVerticalJustificationToTop();
  usageTextProp->SetJustificationToLeft();

  vtkNew<vtkTextMapper> usageTextMapper;
  usageTextMapper->SetInput(
      "- Slice with mouse wheel\n  or Up/Down-Key\n- Zoom with pressed right\n "
      " mouse button while dragging");
  usageTextMapper->SetTextProperty(usageTextProp);

  vtkNew<vtkActor2D> usageTextActor;
  usageTextActor->SetMapper(usageTextMapper);
  usageTextActor->GetPositionCoordinate()
      ->SetCoordinateSystemToNormalizedDisplay();
  usageTextActor->GetPositionCoordinate()->SetValue(0.05, 0.95);

  // create an interactor with our own style (inherit from
  // vtkInteractorStyleImage) in order to catch mousewheel and key events
  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;

  vtkNew<myVtkInteractorStyleImage> myInteractorStyle;

  // make imageviewer2 and sliceTextMapper visible to our interactorstyle
  // to enable slice status message updates when scrolling through the slices
  myInteractorStyle->SetImageViewer(imageViewer);
  myInteractorStyle->SetStatusMapper(sliceTextMapper);
  myInteractorStyle->SetPicker(propPicker);
  myInteractorStyle->SetCurrentRenderer(imageViewer->GetRenderer());

  imageViewer->SetupInteractor(renderWindowInteractor);
  // make the interactor use our own interactorstyle
  // cause SetupInteractor() is defining it's own default interatorstyle
  // this must be called after SetupInteractor()
  renderWindowInteractor->SetInteractorStyle(myInteractorStyle);
  // add slice status message and usage hint message to the renderer
  imageViewer->GetRenderer()->AddActor2D(sliceTextActor);
  imageViewer->GetRenderer()->AddActor2D(usageTextActor);

  // initialize rendering and interaction
  imageViewer->Render();
  imageViewer->GetRenderer()->ResetCamera();
  imageViewer->GetRenderer()->SetBackground(
      colors->GetColor3d("SlateGray").GetData());
  imageViewer->GetRenderWindow()->SetSize(800, 800);
  imageViewer->GetRenderWindow()->SetWindowName("ReadDICOMSeries");
  imageViewer->Render();
  renderWindowInteractor->Start();

  return EXIT_SUCCESS;
}
