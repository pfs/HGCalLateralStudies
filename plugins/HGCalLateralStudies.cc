// -*- C++ -*-
//
// Package:    UserCode/HGCalLateralStudies
// Class:      HGCalLateralStudies
// 
/**\class HGCalLateralStudies HGCalLateralStudies.cc UserCode/HGCalLateralStudies/plugins/HGCalLateralStudies.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Bruno Alves
//         Created:  Mon, 06 May 2019 15:22:22 GMT
//
//


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

#include <vector>
#include <utility> //std::pair

//
// class declaration
//

class HGCalLateralStudies : public edm::stream::EDProducer<> {
   public:
      explicit HGCalLateralStudies(const edm::ParameterSet&);
      ~HGCalLateralStudies();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

   private:
      virtual void beginStream(edm::StreamID) override;
      virtual void produce(edm::Event&, const edm::EventSetup&) override;
      virtual void endStream() override;

      //virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
      //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
      //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

      // ----------member data ---------------------------

  edm::EDGetTokenT<HGCRecHitCollection> recHitsToken;
  typedef std::vector< std::pair<int,int> > CoordCollection;
};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
HGCalLateralStudies::HGCalLateralStudies(const edm::ParameterSet& iConfig)
{
  //src_  = iConfig.getParameter<edm::InputTag>( "src" );
  produces<CoordCollection>("coordinates");

  recHitsToken = consumes<HGCRecHitCollection>(edm::InputTag("HGCalRecHit", "HGCEERecHits"));

/* Examples
   produces<ExampleData2>();

   //if do put with a label
   produces<ExampleData2>("label");
 
   //if you want to put into the Run
   produces<ExampleData2,InRun>();
*/
   //now do what ever other initialization is needed
  
}


HGCalLateralStudies::~HGCalLateralStudies()
{
 
   // do anything here that needs to be done at destruction time
   // (e.g. close files, deallocate resources etc.)

}


//
// member functions
//

// ------------ method called to produce the data  ------------
void
HGCalLateralStudies::produce(edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  std::auto_ptr<CoordCollection> coords(new CoordCollection);

  edm::Handle<HGCRecHitCollection> recHitsHandle;
  iEvent.getByToken(recHitsToken, recHitsHandle);
  const auto &recHits = *recHitsHandle;

  const int size = recHitsHandle->size();
  coords->reserve(size);

  for(HGCRecHitCollection::const_iterator recHit = recHits.begin();
      recHit != recHits.end(); 
      ++recHit) {
    recHit->detid(); 
    //coords->push_back(recHit->detUV());
  }

  iEvent.put(coords);

/* This is an event example
   //Read 'ExampleData' from the Event
   Handle<ExampleData> pIn;
   iEvent.getByLabel("example",pIn);

   //Use the ExampleData to create an ExampleData2 which 
   // is put into the Event
   iEvent.put(std::make_unique<ExampleData2>(*pIn));
*/

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
/*
void
HGCalLateralStudies::beginRun(edm::Run const&, edm::EventSetup const&)
{
}
*/
 
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
