#include "UserCode/HGCalLateralStudies/plugins/HGCalLateralStudies.h"

HGCalLateralStudies::HGCalLateralStudies(const edm::ParameterSet& iConfig):
  recHitsToken(consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit", "HGCEERecHits"))),
  nTotalLayers(iConfig.getParameter<int>("nTotalLayers")),
  nWafers(iConfig.getParameter<int>("nWafersPerLayer")),
  cellFilterCuts(std::pair<iConfig.getParameter<double>("lCellFilterCut"),
		 iConfig.getParameter<double>("hCellFilterCut")>),
  layersAnalysed(iConfig.getParameter<std::vector<int>>("LayersAnalysed")),
  CellUVCollection_name(iConfig.getParameter<std::string>("CellUVCoordinates")),
  WaferUVCollection_name(iConfig.getParameter<std::string>("WaferUVCoordinates"))
{
  produces<CellUVCollection>(CellUVCollection_name);  
  produces<CellUVCollection>(WaferUVCollection_name);  

  waferFilteredMaps.reserve(nTotalLayers);
  layersAnalysed_dirs.reserve(nTotalLayers);
  for(int iLayer=0; iLayer<nTotalLayers; ++iLayer) {
    layersAnalysed_dirs.push_back(fs->mkdir("layer"+std::to_string(iLayer)));
    std::vector<TH2F> h_tmp;
    for(int iWafer=0; iWafer<nWafers; ++iWafer) {
      std::string name_tmp = std::to_string(iWafer);
      TH2F *h_ = layersAnalysed_dirs[iLayer].make<TH2F>(name_tmp.c_str(), name_tmp.c_str(), 
						25, -12, 12, 25, -12, 12);
      h_tmp.push_back(*h_);
    }
    histos.push_back(h_tmp);
  }
}


HGCalLateralStudies::~HGCalLateralStudies()
{
}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HGCalLateralStudies::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::unique_ptr<CellUVCollection> celluv_coords = std::make_unique<CellUVCollection>();
  std::unique_ptr<WaferUVCollection> waferuv_coords = std::make_unique<WaferUVCollection>();

  edm::Handle<HGCRecHitCollection> recHitsHandle;
  iEvent.getByToken(recHitsToken, recHitsHandle);
  const auto &recHits = *recHitsHandle;
  const int size = recHitsHandle->size();
  celluv_coords->reserve(size);
  waferuv_coords->reserve(size);

  /*create maps for one-dimensional wafer identification
    only the layersAnalysed will have a map */
  fillWaferMaps(layersAnalysed, cellFilterCuts);

  for(HGCRecHitCollection::const_iterator recHit = recHits.begin();
      recHit != recHits.end(); 
      ++recHit) {
    HGCSiliconDetId sid(recHit->detid());
    int det_layer = sid.layer();
    //store the data in case the RecHit was measured in one of the user's chosen layersAnalysed
    if(std::find(layersAnalysed.begin(), layersAnalysed.end(), det_layer) != layersAnalysed.end()) {
      std::pair<int,int> cellUV(sid.cellUV());
      std::pair<int,int> waferUV(sid.waferUV());
      int waferId = waferFilteredMaps[det_layer][linearUV(waferUV.first, waferUV.second)];
      histos[det_layer][waferId].Fill(cellUV.first, cellUV.second);
    }
  }
  
  //iEvent.put(std::move(celluv_coords), CellUVCollection_name);
  //iEvent.put(std::move(waferuv_coords), WaferUVCollection_name);

/* this is an EventSetup example
   //Read SetupData from the SetupRecord in the EventSetup
   ESHandle<SetupData> pSetup;
   iSetup.get<SetupRecord>().get(pSetup);
*/
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void
HGCalLateralStudies::beginStream(edm::StreamID)
{
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void
HGCalLateralStudies::endStream() {
}

// ------------ method called when starting to processes a run  ------------
void
HGCalLateralStudies::beginRun(edm::Run const&, edm::EventSetup const&)
{
  edm::ESHandle<CaloGeometry> geom;
  es.get<CaloGeometryRecord>().get(geom);

  if(myDet_==DetId::HGCalEE || myDet_==DetId::HGCalHSi)
    gHGCal_ = dynamic_cast<const HGCalGeometry*>(geom->getSubdetectorGeometry(myDet_, mySubDet_));
  else {
    gHGCal = 0;
    throw std::domain_error("wrong detector");
  }
}

 
// ------------ method called when ending the processing of a run  ------------
/*
void
HGCalLateralStudies::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when starting to processes a luminosity block  ------------
/*
void
HGCalLateralStudies::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
HGCalLateralStudies::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/
 
// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HGCalLateralStudies::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(HGCalLateralStudies);
