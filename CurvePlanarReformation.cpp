vtkActor* pissSurgeryPlanWindow::doCurvePlanReformation(vtkPoints* route){

    vtkSCurveSpline *xSpline= vtkSCurveSpline::New();
    vtkSCurveSpline *ySpline= vtkSCurveSpline::New();
    vtkSCurveSpline *zSpline= vtkSCurveSpline::New();
    vtkParametricSpline *spline = vtkParametricSpline::New();
    vtkParametricFunctionSource *functionSource = vtkParametricFunctionSource::New();

    vtkSplineDrivenImageSlicer *reslicer = vtkSplineDrivenImageSlicer::New();
    vtkImageAppend *append = vtkImageAppend::New();

    //! cpr slice
    vtkPolyDataMapper *cprmapper = vtkPolyDataMapper::New();
    vtkActor *cpractor = vtkActor::New();

    vtkPlane *pPlane = vtkPlane::New();
    vtkCutter *pCut = vtkCutter::New();
    vtkImageShiftScale *m_pShift = vtkImageShiftScale::New();

    //! cpr volume
    vtkFixedPointVolumeRayCastMapper *cprVolumeMapper = vtkFixedPointVolumeRayCastMapper::New();
    vtkVolume *cprvolume = vtkVolume::New();
    vtkColorTransferFunction *cprcolorTranFun = vtkColorTransferFunction::New();
    vtkVolumeProperty *cprVolumeproperty = vtkVolumeProperty::New();
    vtkPiecewiseFunction *cprPieceFun = vtkPiecewiseFunction::New();

    spline->SetXSpline(xSpline);
    spline->SetYSpline(ySpline);
    spline->SetZSpline(zSpline);
    spline->SetPoints(route);
    functionSource->SetParametricFunction(spline);
    functionSource->SetUResolution(200);
    functionSource->SetVResolution(200);
    functionSource->SetWResolution(200);
    functionSource->Update();

    reslicer->SetInputData(patientHandling->getMraImageToBeDisplayed());
    reslicer->SetPathConnection(functionSource->GetOutputPort());
    reslicer->SetSliceSpacing(0.2, 0.1);
    reslicer->SetSliceThickness(0.8);
    reslicer->SetSliceExtent(200, 200);
    reslicer->SetOffsetPoint(30);

    long long nbPoints = functionSource->GetOutput()->GetNumberOfPoints();
    for( int ptId = 0; ptId < nbPoints; ptId++){
        reslicer->SetOffsetPoint(ptId);
        reslicer->Update();
         vtkImageData *tempSlice = vtkImageData::New();
        tempSlice->DeepCopy( reslicer->GetOutput( 0 ));
        append->AddInputData(tempSlice);
    }
    append->SetAppendAxis(2);
    append->Update();

//    vtkSmartPointer<vtkMetaImageWriter> writer =
//         vtkSmartPointer<vtkMetaImageWriter>::New();
//      writer->SetInputConnection(append->GetOutputPort());
//      writer->SetFileName("C:\\Users\\cheng\\Desktop\\hehe\\hehe.mhd");
//      writer->SetRAWFileName("C:\\Users\\cheng\\Desktop\\hehe\\hehe.raw");
//      writer->Write();

    //! TODO append->GetOutput()
    cprVolumeMapper->SetInputConnection(append->GetOutputPort());
    cprcolorTranFun->AddRGBSegment(0,1,1,1,255,1,1,1);
    cprPieceFun->AddSegment(0,0,3000,1);
    cprPieceFun->AddPoint(20,0.2);
    cprPieceFun->AddPoint(80,0.5);
    cprPieceFun->AddPoint(120,0.7);
    cprPieceFun->AddPoint(200,0.9);
    cprPieceFun->ClampingOff();
    cprVolumeMapper->SetBlendModeToMaximumIntensity();
    cprVolumeproperty->SetColor(cprcolorTranFun);
    cprVolumeproperty->SetScalarOpacity(cprPieceFun);
    cprVolumeproperty->SetInterpolationTypeToLinear();
    cprVolumeproperty->ShadeOff();
    cprvolume->SetProperty(cprVolumeproperty);
    cprvolume->SetMapper(cprVolumeMapper);

    double range[2];
    patientHandling->getMraImageToBeDisplayed()->GetScalarRange(range);
    m_pShift->SetShift(-1.0*range[0]);
    m_pShift->SetScale(255.0/(range[1]-range[0]));
    m_pShift->SetOutputScalarTypeToUnsignedChar();
    m_pShift->SetInputConnection(append->GetOutputPort());
    m_pShift->ReleaseDataFlagOff();
    m_pShift->Update();

    pPlane->SetOrigin(cprvolume->GetCenter());
    pPlane->SetNormal(1,1,0);
    pCut->SetCutFunction(pPlane);
    pCut->SetInputConnection(m_pShift->GetOutputPort());
    pCut->Update();

    cprmapper->SetInputData(pCut->GetOutput());
    cpractor->SetMapper(cprmapper);
    return cpractor;
}
