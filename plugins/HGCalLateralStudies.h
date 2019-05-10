// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/HGCRecHit/interface/HGCRecHit.h"
#include "DataFormats/HGCRecHit/interface/HGCRecHitCollections.h"
#include "DataFormats/ForwardDetId/interface/HGCSiliconDetId.h"
#include "DataFormats/GeometryVector/interface/GlobalPoint.h"
#include "FWCore/Utilities/interface/EDGetToken.h"
#include "FWCore/Utilities/interface/EDPutToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "Geometry/HGCalGeometry/interface/HGCalGeometry.h"

#include <vector>
#include <utility> //std::pair
#include <algorithm> //std::find
#include "TH2F.h"

//
// class declaration
//
class HGCalLateralStudies : public edm::stream::EDProducer<> {
public:
  explicit HGCalLateralStudies(const edm::ParameterSet&);
  ~HGCalLateralStudies();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);
  
private:
  //typedefs
  typedef std::unordered_map<int,int> waferFilteredMap;

  //variables
  const edm::EDGetTokenT<HGCRecHitCollection> recHitsToken_;  
  const edm::Service<TFileService> fs_;
  const HGCalGeometry* gHGCal_;
  DetId::Detector myDet_; 
  ForwardSubdetector mySubDet_;
  const int nTotalLayers_;
  const int nWafers_;
  const std::pair<double, double> cellFilterCuts_;
  std::vector<TFileDirectory> layersAnalysedDirs_; //one subdir per layer even if it is not used
  const std::vector<int> layersAnalysed_;
  std::vector< std::vector<TH2F> > histos_; //1d: layersAnalysed; 2d: wafers
  std::vector<waferFilteredMap> waferFilteredMaps_; //one waferFilteredMap per analysed layer

  //variables (outputs)
  const std::string CellUVCollection_name_;
  typedef std::vector< std::pair<int,int> > CellUVCollection_;
  const std::string WaferUVCollection_name_;
  typedef std::vector< std::pair<int,int> > WaferUVCollection_;

  //functions
  virtual void beginStream(edm::StreamID) override;
  virtual void produce(edm::Event&, const edm::EventSetup&) override;
  virtual void endStream() override;
  //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  const int linearUV(int u, int v) {return v*100+u;}
  const HGCalGeometry* getGeometry();
  
  //templates
  template <class T> 
    bool cellFilter(GlobalPoint p, std::pair<T,T> cuts) {
    return p.mag() < cuts.first || p.mag() > cuts.second;
  }
  template <class T>
    void fillWaferMap(int layer, std::pair<T,T> cuts) {
    /*Fills the wafer maps such that for all analysed layers conversion between (u,v)
      and a linear index [0;nTotalLayers-1] is readily available and stored in the class.
      The filter selects only the (u,v) pairs that correspond to wafers close to the center.
    */
    int pos(0);
    std::vector<DetId>& ids = gHGCal_->getValidDetIds();
    for(std::vector<DetId>::iterator it = ids.begin(); it != ids.end(); ++it) {
      GlobalPoint point = gHGCal_->getPosition(*it);
      //filter
      if(cellFilter(point, cuts)) {
	HGCSiliconDetId sid(*it);
	std::pair<int,int> uv = sid.cellUV();
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
  template <class T> 
    void fillWaferMaps(std::vector<int> layers, std::pair<T,T> cuts) {
    for(std::vector<int>::iterator it = layers.begin(); it!=layers.end(); ++it)
      fillWaferMap(*it, cuts);
  }
};
