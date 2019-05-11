#ifndef HGCalLateralStudies_h
#define HGCalLateralStudies_h

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
#include "Geometry/CaloGeometry/interface/CaloGeometry.h"
#include "Geometry/Records/interface/CaloGeometryRecord.h"
#include "RecoLocalCalo/HGCalRecAlgos/interface/RecHitTools.h"

#include <iostream>
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
  typedef unsigned int int_layer;
  typedef std::unordered_map<int,int> waferFilteredMap;

  //variables
  const edm::EDGetTokenT<HGCRecHitCollection> recHitsToken_;  
  const edm::Service<TFileService> fs_;
  const HGCalGeometry* gHGCal_;
  DetId::Detector myDet_; 
  ForwardSubdetector mySubDet_;
  hgcal::RecHitTools recHitTools_;
  int_layer lastLayerEE_= 99;
  int_layer lastLayerFH_= 99;
  const int nTotalLayers_;
  const int nWafers_;
  const std::pair<double, double> cellFilterCuts_;
  std::vector<TFileDirectory> layersAnalysedDirs_; //one subdir per layer even if it is not used
  const std::vector<int_layer> layersAnalysed_;
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
  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  int linearUV(int u, int v) {return v*100+u;}
  void setDetector(int_layer);
  void recHitToolsSetup(const edm::EventSetup&);
  bool cellFilter(GlobalPoint);
  void fillWaferMap(const int_layer);
  void fillWaferMaps(const std::vector<int_layer>);
};

#endif //HGCalLateralStudies_h
