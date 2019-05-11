#include "UserCode/HGCalLateralStudies/plugins/HGCalLateralStudies.h"

HGCalLateralStudies::HGCalLateralStudies(const edm::ParameterSet& iConfig):
  recHitsToken_(consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit", "HGCEERecHits"))),
  nTotalLayers_(iConfig.getParameter<int>("nTotalLayers")),
  nWafers_(iConfig.getParameter<int>("nWafersPerLayer")),
  cellFilterCuts_ (std::make_pair(iConfig.getParameter<double>("lCellFilterCut"),
				  iConfig.getParameter<double>("hCellFilterCut"))),
  layersAnalysed_(iConfig.getParameter<std::vector<int_layer>>("LayersAnalysed")),
  CellUVCollection_name_(iConfig.getParameter<std::string>("CellUVCoordinates")),
  WaferUVCollection_name_(iConfig.getParameter<std::string>("WaferUVCoordinates"))
{
  produces<CellUVCollection_>(CellUVCollection_name_);  
  produces<CellUVCollection_>(WaferUVCollection_name_);  

  waferFilteredMaps_.reserve(nTotalLayers_);
  layersAnalysedDirs_.reserve(nTotalLayers_);
  for(int iLayer=0; iLayer<nTotalLayers_; ++iLayer) {
    layersAnalysedDirs_.push_back(fs_->mkdir("layer"+std::to_string(iLayer)));
    std::vector<TH2F> h_tmp;
    for(int iWafer=0; iWafer<nWafers_; ++iWafer) {
      std::string name_tmp = std::to_string(iWafer);
      TH2F *h_ = layersAnalysedDirs_[iLayer].make<TH2F>(name_tmp.c_str(), name_tmp.c_str(), 
						       25, -12, 12, 25, -12, 12);
      h_tmp.push_back(*h_);
    }
    histos_.push_back(h_tmp);
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
  std::unique_ptr<CellUVCollection_> celluv_coords = std::make_unique<CellUVCollection_>();
  std::unique_ptr<WaferUVCollection_> waferuv_coords = std::make_unique<WaferUVCollection_>();

  edm::Handle<HGCRecHitCollection> recHitsHandle;
  iEvent.getByToken(recHitsToken_, recHitsHandle);
  const auto &recHits = *recHitsHandle;
  const int size = recHitsHandle->size();
  celluv_coords->reserve(size);
  waferuv_coords->reserve(size);

  recHitToolsSetup(iSetup);

  /*create maps for one-dimensional wafer identification
    only the layersAnalysed will have a map */
  fillWaferMaps(layersAnalysed_);

  for(const auto &recHit : recHits){
    HGCSiliconDetId sid(recHit.detid());
    int_layer det_layer = static_cast<int_layer>(sid.layer());
    //store the data in case the RecHit was measured in one of the user's chosen layersAnalysed
    if(std::find(layersAnalysed_.begin(), layersAnalysed_.end(), det_layer) != layersAnalysed_.end()) {
      std::pair<int,int> cellUV(sid.cellUV());
      std::pair<int,int> waferUV(sid.waferUV());
      int waferId = waferFilteredMaps_[det_layer][linearUV(waferUV.first, waferUV.second)];
      histos_[det_layer][waferId].Fill(cellUV.first, cellUV.second);
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
HGCalLateralStudies::beginRun(edm::Run const&, edm::EventSetup const& es)
{
  edm::ESHandle<CaloGeometry> geom;
  es.get<CaloGeometryRecord>().get(geom);

  if(myDet_==DetId::HGCalEE || myDet_==DetId::HGCalHSi)
    gHGCal_ = dynamic_cast<const HGCalGeometry*>(geom->getSubdetectorGeometry(myDet_, mySubDet_));
  else {
    gHGCal_ = 0;
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
 
void HGCalLateralStudies::setDetector(int_layer layer) {
  if(layer > lastLayerEE_)
    throw std::domain_error("choose another layer");
  else {
    myDet_=DetId::HGCalEE;
    mySubDet_=ForwardSubdetector::ForwardEmpty;
  }
}

void HGCalLateralStudies::recHitToolsSetup(const edm::EventSetup& setup) {
  recHitTools_.getEventSetup(setup);
  //without the check it runs once per event
  if(lastLayerEE_== 99)
    lastLayerEE_ = recHitTools_.lastLayerEE();
  if(lastLayerFH_== 99)
    lastLayerFH_ = recHitTools_.lastLayerFH();
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
HGCalLateralStudies::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.addUntracked<int>("nTotalLayers");
  desc.addUntracked<int>("nWafersPerLayer");
  desc.addUntracked<double>("lCellFilterCut");
  desc.addUntracked<double>("hCellFilterCut");
  desc.addUntracked< std::vector<int> >("layersAnalysed");
  desc.addUntracked<std::string>("CellUVCoordinates");
  desc.addUntracked<std::string>("WaferUVCoordinates");
}

bool HGCalLateralStudies::cellFilter(GlobalPoint p) {
  return p.mag() < cellFilterCuts_.first || p.mag() > cellFilterCuts_.second;
}

void HGCalLateralStudies::fillWaferMap(int_layer layer) {
  /*Fills the wafer maps such that for all analysed layers conversion between (u,v)
    and a linear index [0;nTotalLayers-1] is readily available and stored in the class.
    The filter selects only the (u,v) pairs that correspond to wafers close to the center.
  */
  int pos(0);
  const std::vector<DetId>& ids = gHGCal_->getValidDetIds();
  for(const auto &it : ids) {
    GlobalPoint point = gHGCal_->getPosition(it);
    if(cellFilter(point)) {
      HGCSiliconDetId sid(it);
      std::pair<int,int> uv = sid.waferUV();
      //check that the (u,v) pair was not introduced before
      if (std::find(waferFilteredMaps_[layer].begin(), 
		    waferFilteredMaps_[layer].end(), 
		    uv) == waferFilteredMaps_[layer].end()) {
	waferFilteredMaps_[layer].insert( linearUV(uv.first, uv.second), pos );
	++pos;
      }
    }
  }
}

void HGCalLateralStudies::fillWaferMaps(const std::vector<int_layer> layers) {
  for(std::vector<int_layer>::const_iterator it = layers.begin(); it!=layers.end(); ++it) {
    setDetector(*it);
    fillWaferMap(*it);
  }
}
//define this as a plug-in
DEFINE_FWK_MODULE(HGCalLateralStudies);
