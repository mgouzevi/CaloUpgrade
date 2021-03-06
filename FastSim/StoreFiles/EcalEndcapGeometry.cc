#include "Geometry/CaloGeometry/interface/CaloGenericDetId.h"
#include "Geometry/EcalAlgo/interface/EcalEndcapGeometry.h"
#include "Geometry/CaloGeometry/interface/CaloCellGeometry.h"
#include "Geometry/CaloGeometry/interface/TruncatedPyramid.h"
#include "FWCore/Utilities/interface/Exception.h"
#include <CLHEP/Geometry/Point3D.h>
#include <CLHEP/Geometry/Plane3D.h>

typedef CaloCellGeometry::CCGFloat CCGFloat ;
typedef CaloCellGeometry::Pt3D     Pt3D     ;
typedef CaloCellGeometry::Pt3DVec  Pt3DVec  ;
typedef HepGeom::Plane3D<CCGFloat> Pl3D     ;

EcalEndcapGeometry::EcalEndcapGeometry( void )
  : _nnmods( 316 ),
    _nncrys( 25 ),
    zeP( 0. ),
    zeN( 0. ),
    m_wref( 0. ),
    m_del( 0. ),
    m_nref( 0 ),
    m_borderMgr( 0 ),
    m_borderPtrVec( 0 ),
    m_avgZ( -1 ),
    m_cellVec( k_NumberOfCellsForCorners )
{
  m_xlo[0] = 999.;
  m_xlo[1] = 999.;
  m_xhi[0] = -999.;
  m_xhi[1] = -999.;
  m_ylo[0] = 999.;
  m_ylo[1] = 999.;
  m_yhi[0] = -999.;
  m_yhi[1] = -999.;
  m_xoff[0] = 0.;
  m_xoff[1] = 0.;
  m_yoff[0] = 0.;
  m_yoff[0] = 0.;
}

EcalEndcapGeometry::~EcalEndcapGeometry() 
{
   delete m_borderPtrVec ;
   delete m_borderMgr ;
}

unsigned int
EcalEndcapGeometry::alignmentTransformIndexLocal( const DetId& id )
{
   const CaloGenericDetId gid ( id ) ;

   assert( gid.isEE() ) ;
   unsigned int index ( EEDetId(id).ix()/51 + ( EEDetId(id).zside()<0 ? 0 : 2 ) ) ;

   return index ;
}

DetId 
EcalEndcapGeometry::detIdFromLocalAlignmentIndex( unsigned int iLoc )
{
   return EEDetId( 20 + 50*( iLoc%2 ), 50, 2*( iLoc/2 ) - 1 ) ;
}

unsigned int
EcalEndcapGeometry::alignmentTransformIndexGlobal( const DetId& /*id*/ )
{
   return (unsigned int)DetId::Ecal - 1 ;
}

void 
EcalEndcapGeometry::initializeParms()
{
  zeP=0.;
  zeN=0.;
  unsigned nP=0;
  unsigned nN=0;
  m_nref = 0 ;

  for( uint32_t i ( 0 ) ; i != m_cellVec.size()/2 ; ++i )
  {
     const CaloCellGeometry* cell ( cellGeomPtr(i) ) ;
     if( 0 != cell )
     {
	const CCGFloat z ( cell->getPosition().z() ) ;
	if(z>0.)
	{
	   zeP+=z;
	   ++nP;
	}
	else
	{
	   zeN+=z;
	   ++nN;
	}
	const EEDetId myId ( EEDetId::detIdFromDenseIndex(i) ) ;
	const unsigned int ix ( myId.ix() ) ;
	const unsigned int iy ( myId.iy() ) ;
	if( ix > m_nref ) m_nref = ix ;
	if( iy > m_nref ) m_nref = iy ;
     }
  }
  if( 0 < nP ) zeP/=(CCGFloat)nP;
  if( 0 < nN ) zeN/=(CCGFloat)nN;

  m_xlo[0] =  999 ;
  m_xhi[0] = -999 ;
  m_ylo[0] =  999 ;
  m_yhi[0] = -999 ;
  m_xlo[1] =  999 ;
  m_xhi[1] = -999 ;
  m_ylo[1] =  999 ;
  m_yhi[1] = -999 ;
  for( uint32_t i ( 0 ) ; i != m_cellVec.size()/2 ; ++i )
  {
     const CaloCellGeometry* cell ( cellGeomPtr(i) ) ;
     if( 0 != cell )
     {
	const GlobalPoint p ( cell->getPosition()  ) ;
	const CCGFloat z ( p.z() ) ;
	const CCGFloat zz ( 0 > z ? zeN : zeP ) ;
	const CCGFloat x ( p.x()*zz/z ) ;
	const CCGFloat y ( p.y()*zz/z ) ;

	if( 0 > z && x < m_xlo[0] ) m_xlo[0] = x ;
	if( 0 < z && x < m_xlo[1] ) m_xlo[1] = x ;
	if( 0 > z && y < m_ylo[0] ) m_ylo[0] = y ;
	if( 0 < z && y < m_ylo[1] ) m_ylo[1] = y ;
     
	if( 0 > z && x > m_xhi[0] ) m_xhi[0] = x ;
	if( 0 < z && x > m_xhi[1] ) m_xhi[1] = x ;
	if( 0 > z && y > m_yhi[0] ) m_yhi[0] = y ;
	if( 0 < z && y > m_yhi[1] ) m_yhi[1] = y ;
     }
  }

  m_xoff[0] = ( m_xhi[0] + m_xlo[0] )/2. ;
  m_xoff[1] = ( m_xhi[1] + m_xlo[1] )/2. ;
  m_yoff[0] = ( m_yhi[0] + m_ylo[0] )/2. ;
  m_yoff[1] = ( m_yhi[1] + m_ylo[1] )/2. ;

  m_del = ( m_xhi[0] - m_xlo[0] + m_xhi[1] - m_xlo[1] +
	    m_yhi[0] - m_ylo[0] + m_yhi[1] - m_ylo[1]   ) ;

  if( 1 != m_nref ) m_wref = m_del/(4.*(m_nref-1)) ;

  m_xlo[0] -= m_wref/2 ;
  m_xlo[1] -= m_wref/2 ;
  m_xhi[0] += m_wref/2 ;
  m_xhi[1] += m_wref/2 ;

  m_ylo[0] -= m_wref/2 ;
  m_ylo[1] -= m_wref/2 ;
  m_yhi[0] += m_wref/2 ;
  m_yhi[1] += m_wref/2 ;

  m_del += m_wref ;
/*
  std::cout<<"zeP="<<zeP<<", zeN="<<zeN<<", nP="<<nP<<", nN="<<nN<<std::endl ;

  std::cout<<"xlo[0]="<<m_xlo[0]<<", xlo[1]="<<m_xlo[1]<<", xhi[0]="<<m_xhi[0]<<", xhi[1]="<<m_xhi[1]
	   <<"\nylo[0]="<<m_ylo[0]<<", ylo[1]="<<m_ylo[1]<<", yhi[0]="<<m_yhi[0]<<", yhi[1]="<<m_yhi[1]<<std::endl ;

  std::cout<<"xoff[0]="<<m_xoff[0]<<", xoff[1]"<<m_xoff[1]<<", yoff[0]="<<m_yoff[0]<<", yoff[1]"<<m_yoff[1]<<std::endl ;

  std::cout<<"nref="<<m_nref<<", m_wref="<<m_wref<<std::endl ;
*/  
}


unsigned int 
EcalEndcapGeometry::xindex( CCGFloat x,
			    CCGFloat z ) const
{
   const CCGFloat xlo ( 0 > z ? m_xlo[0]  : m_xlo[1]  ) ;
   const int i ( 1 + int( ( x - xlo )/m_wref ) ) ;

   return ( 1 > i ? 1 :
	    ( m_nref < (unsigned int) i ? m_nref : (unsigned int) i ) ) ;

}

unsigned int 
EcalEndcapGeometry::yindex( CCGFloat y,
			    CCGFloat z  ) const
{
   const CCGFloat ylo ( 0 > z ? m_ylo[0]  : m_ylo[1]  ) ;
   const int i ( 1 + int( ( y - ylo )/m_wref ) ) ;

   return ( 1 > i ? 1 :
	    ( m_nref < (unsigned int) i ? m_nref : (unsigned int) i ) ) ;
}

DetId 
EcalEndcapGeometry::getClosestCell( const GlobalPoint& r ) const 
{
   CCGFloat closest ( 1.e9 ) ;
   DetId returnId ( 0 ) ;
//   const CCGFloat eta ( r.eta() ) ;
//   const CCGFloat phi ( r.phi() ) ;
   const CaloSubdetectorGeometry::DetIdSet dis ( getCells( r, M_PI/50. ) ) ;
   for( CaloSubdetectorGeometry::DetIdSet::const_iterator id ( dis.begin() ); id != dis.end() ; ++id )
   {
      const unsigned int i ( CaloGenericDetId( *id ).denseIndex() ) ;
      const CaloCellGeometry* cell ( cellGeomPtr( i ) ) ;
      if( 0 != cell )
      {
	 if( cell->inside( r ) ) 
	 {
	    returnId = *id ;
	    break ;
	 }
	 const GlobalPoint& p ( cell->getPosition() ) ;
//	 const CCGFloat eta0 ( p.eta() ) ;
//	 const CCGFloat phi0 ( p.phi() ) ;
//	 const CCGFloat dR2 ( reco::deltaR2( eta0, phi0, eta, phi ) ) ;
	 const CCGFloat dR2 ( (p-r).mag2() ) ;
	 if( dR2 < closest ) 
	 {
	    closest = dR2 ;
	    returnId = *id ;
	 }
      }
   }
   return returnId ;
}

/*
EEDetId 
EcalEndcapGeometry::gId( float x, 
			 float y, 
			 float z ) const
{
   const CCGFloat     fac ( fabs( ( 0 > z ? zeN : zeP )/z ) ) ;
   const unsigned int ix  ( xindex( x*fac, z ) ) ; 
   const unsigned int iy  ( yindex( y*fac, z ) ) ; 
   const unsigned int iz  ( z>0 ? 1 : -1 ) ;

   if( EEDetId::validDetId( ix, iy, iz ) ) 
   {
      return EEDetId( ix, iy, iz ) ; // first try is on target
   }
   else // try nearby coordinates, spiraling out from center
   {
      for( unsigned int i ( 1 ) ; i != 6 ; ++i )
      {
	 for( unsigned int k ( 0 ) ; k != 8 ; ++k )
	 {
	    const int jx ( 0 == k || 4 == k || 5 == k ? +i :
			   ( 1 == k || 5 < k ? -i : 0 ) ) ;
	    const int jy ( 2 == k || 4 == k || 6 == k ? +i :
			   ( 3 == k || 5 == k || 7 == k ? -i : 0 ) ) ;
	    if( EEDetId::validDetId( ix + jx, iy + jy, iz ) ) 
	    {
	       return EEDetId( ix + jx, iy + jy, iz ) ;
	    }
	 }
      }
   }
   return EEDetId() ; // nowhere near any crystal
}


// Get closest cell, etc...
DetId 
EcalEndcapGeometry::getClosestCell( const GlobalPoint& r ) const 
{
   try
   {
      EEDetId mycellID ( gId( r.x(), r.y(), r.z() ) ) ; // educated guess

      if( EEDetId::validDetId( mycellID.ix(), 
			       mycellID.iy(),
			       mycellID.zside() ) )
      {
	 // now get points in convenient ordering

	 Pt3D A;
	 Pt3D B;
	 Pt3D C;
	 Pt3D point(r.x(),r.y(),r.z());
	 // D.K. : equation of plane : AA*x+BB*y+CC*z+DD=0;
	 // finding equation for each edge
	 
	 // ================================================================
	 CCGFloat x,y,z;
	 unsigned offset=0;
	 int zsign=1;
	 //================================================================
	 std::vector<CCGFloat> SS;
      
	 // compute the distance of the point with respect of the 4 crystal lateral planes

	 
	 if( 0 != getGeometry(mycellID) )
	 {
	    const GlobalPoint& myPosition=getGeometry(mycellID)->getPosition();
	 
	    x=myPosition.x();
	    y=myPosition.y();
	    z=myPosition.z();
	 
	    offset=0;
	    // This will disappear when Andre has applied his fix
	    zsign=1;
	 
	    if(z>0)
	    {
	       if(x>0&&y>0)
		  offset=1;
	       else  if(x<0&&y>0)
		  offset=2;
	       else if(x>0&&y<0)
		  offset=0;
	       else if (x<0&&y<0)
		  offset=3;
	       zsign=1;
	    }
	    else
	    {
	       if(x>0&&y>0)
		  offset=3;
	       else if(x<0&&y>0)
		  offset=2;
	       else if(x>0&&y<0)
		  offset=0;
	       else if(x<0&&y<0)
		  offset=1;
	       zsign=-1;
	    }
	    std::vector<GlobalPoint> corners;
	    corners.clear();
	    corners.resize(8);
	    for(unsigned ic=0;ic<4;++ic)
	    {
	       corners[ic]=getGeometry(mycellID)->getCorners()[(unsigned)((zsign*ic+offset)%4)];
	       corners[4+ic]=getGeometry(mycellID)->getCorners()[(unsigned)(4+(zsign*ic+offset)%4)];
	    }
	    
	    for (short i=0; i < 4 ; ++i)
	    {
	       A = Pt3D(corners[i%4].x(),corners[i%4].y(),corners[i%4].z());
	       B = Pt3D(corners[(i+1)%4].x(),corners[(i+1)%4].y(),corners[(i+1)%4].z());
	       C = Pt3D(corners[4+(i+1)%4].x(),corners[4+(i+1)%4].y(),corners[4+(i+1)%4].z());
	       Pl3D plane(A,B,C);
	       plane.normalize();
	       CCGFloat distance = plane.distance(point);
	       if (corners[0].z()<0.) distance=-distance;
	       SS.push_back(distance);
	    }
	 
	    // Only one move in necessary direction
	 
	    const bool yout ( 0 > SS[0]*SS[2] ) ;
	    const bool xout ( 0 > SS[1]*SS[3] ) ;
	 
	    if( yout || xout )
	    {
	       const int ydel ( !yout ? 0 :  ( 0 < SS[0] ? -1 : 1 ) ) ;
	       const int xdel ( !xout ? 0 :  ( 0 < SS[1] ? -1 : 1 ) ) ;
	       const unsigned int ix ( mycellID.ix() + xdel ) ;
	       const unsigned int iy ( mycellID.iy() + ydel ) ;
	       const unsigned int iz ( mycellID.zside()     ) ;
	       if( EEDetId::validDetId( ix, iy, iz ) ) 
		  mycellID = EEDetId( ix, iy, iz ) ;
	    }
  
	    return mycellID;
	 }
	 return DetId(0);
      }
   }
   catch ( cms::Exception &e ) 
   { 
      return DetId(0);
   }
   return DetId(0);
}
*/
CaloSubdetectorGeometry::DetIdSet 
EcalEndcapGeometry::getCells( const GlobalPoint& r, 
			      double             dR ) const 
{
//   std::cout<<"entering getCells"<<std::endl;
   CaloSubdetectorGeometry::DetIdSet dis ; // return object
   if( 0.000001 < dR )
   {
      if( dR > M_PI/2. ) // this code assumes small dR
      {
	 dis = CaloSubdetectorGeometry::getCells( r, dR ) ; // base class version
      }
      else
      {
	 const double dR2  ( dR*dR ) ;
	 const double reta ( r.eta() ) ;
	 const double rphi ( r.phi() ) ;
	 const double rx   ( r.x() ) ;
	 const double ry   ( r.y() ) ;
	 const double rz   ( r.z() ) ;
	 const double fac  ( fabs( zeP/rz ) ) ;
	 const double xx   ( rx*fac ) ; // xyz at endcap z
	 const double yy   ( ry*fac ) ; 
	 const double zz   ( rz*fac ) ; 

	 const double xang  ( atan( xx/zz ) ) ;
	 const double lowX  ( zz>0 ? zz*tan( xang - dR ) : zz*tan( xang + dR ) ) ;
	 const double highX ( zz>0 ? zz*tan( xang + dR ) : zz*tan( xang - dR ) ) ;
	 const double yang  ( atan( yy/zz ) ) ;
	 const double lowY  ( zz>0 ? zz*tan( yang - dR ) : zz*tan( yang + dR ) ) ;
	 const double highY ( zz>0 ? zz*tan( yang + dR ) : zz*tan( yang - dR ) ) ;

	 const double refxlo ( 0 > rz ? m_xlo[0] : m_xlo[1] ) ;
	 const double refxhi ( 0 > rz ? m_xhi[0] : m_xhi[1] ) ;
	 const double refylo ( 0 > rz ? m_ylo[0] : m_ylo[1] ) ;
	 const double refyhi ( 0 > rz ? m_yhi[0] : m_yhi[1] ) ;

	 if( lowX  <  refxhi &&   // proceed if any possible overlap with the endcap
	     lowY  <  refyhi &&
	     highX >  refxlo &&
	     highY >  refylo    )
	 {
	    const int ix_ctr ( xindex( xx, rz ) ) ;
	    const int iy_ctr ( yindex( yy, rz ) ) ;
	    const int iz     ( rz>0 ? 1 : -1 ) ;
	    
	    const int ix_hi  ( ix_ctr + int( ( highX - xx )/m_wref ) + 2 ) ;
	    const int ix_lo  ( ix_ctr - int( ( xx - lowX  )/m_wref ) - 2 ) ;
	    
	    const int iy_hi  ( iy_ctr + int( ( highY - yy )/m_wref ) + 2 ) ;
	    const int iy_lo  ( iy_ctr - int( ( yy - lowY  )/m_wref ) - 2 ) ;
	    
	    for( int kx ( ix_lo ) ; kx <= ix_hi ; ++kx ) 
	    {
	       if( kx >  0      && 
		   kx <= (int) m_nref    )
	       {
		  for( int ky ( iy_lo ) ; ky <= iy_hi ; ++ky ) 
		  {
		     if( ky >  0      && 
			 ky <= (int) m_nref    )
		     {
			for( int kz (1); kz<=2; ++kz)
			{
			   if( EEDetId::validDetId( kx, ky, iz*kz ) ) // reject invalid ids
			   {
			      const EEDetId id ( kx, ky, iz*kz ) ;
			      const CaloCellGeometry* cell ( getGeometry( id ) );
			      if( 0 != cell )
			      {
				 const GlobalPoint& p    ( cell->getPosition() ) ;
				 const double       eta  ( p.eta() ) ;
				 const double       phi  ( p.phi() ) ;
				 const double distance2 ( reco::deltaR2( eta, phi, reta, rphi ) ) ;
				 if( distance2 < dR2 ) 
				 {
//				    std::cout<<"getCells:adding id="<<id<<std::endl;
				    dis.insert( id ) ;
				 }
			      }
			   }
			}
		     }
		  }
	       }
	    }
	 }
      }
   }
   return dis;
}

const EcalEndcapGeometry::OrderedListOfEBDetId*
EcalEndcapGeometry::getClosestBarrelCells( EEDetId id ) const
{
   OrderedListOfEBDetId* ptr ( 0 ) ;
   if( 0 != id.rawId() &&
       0 != getGeometry( id ) )
   {
      const float phi ( 370. +
			getGeometry( id )->getPosition().phi().degrees() );
      const int iPhi ( 1 + int(phi)%360 ) ;
      const int iz ( id.zside() ) ;
      if( 0 == m_borderMgr )
      {
	 m_borderMgr = new EZMgrFL<EBDetId>( 720*9, 9 ) ;
      }
      if( 0 == m_borderPtrVec )
      {
	 m_borderPtrVec = new VecOrdListEBDetIdPtr() ;
	 m_borderPtrVec->reserve( 720 ) ;
	 for( unsigned int i ( 0 ) ; i != 720 ; ++i )
	 {
	    const int kz     ( 360>i ? -1 : 1 ) ;
	    const int iEta   ( kz*85 ) ;
	    const int iEtam1 ( kz*84 ) ;
	    const int iEtam2 ( kz*83 ) ;
	    const int jPhi   ( i%360 + 1 ) ;
	    OrderedListOfEBDetId& olist ( *new OrderedListOfEBDetId( m_borderMgr ) );
	    olist[0]=EBDetId( iEta  ,        jPhi     ) ;
	    olist[1]=EBDetId( iEta  , myPhi( jPhi+1 ) ) ;
	    olist[2]=EBDetId( iEta  , myPhi( jPhi-1 ) ) ;
	    olist[3]=EBDetId( iEtam1,        jPhi     ) ;
	    olist[4]=EBDetId( iEtam1, myPhi( jPhi+1 ) ) ;
	    olist[5]=EBDetId( iEtam1, myPhi( jPhi-1 ) ) ;
	    olist[6]=EBDetId( iEta  , myPhi( jPhi+2 ) ) ;
	    olist[7]=EBDetId( iEta  , myPhi( jPhi-2 ) ) ;
	    olist[8]=EBDetId( iEtam2,        jPhi     ) ;
	    m_borderPtrVec->push_back( &olist ) ;
	 }
      }
      ptr = (*m_borderPtrVec)[ ( iPhi - 1 ) + ( 0>iz ? 0 : 360 ) ] ;
   }
   return ptr ;
}

void
EcalEndcapGeometry::localCorners( Pt3DVec&        lc  ,
				  const CCGFloat* pv  ,
				  unsigned int   /*i*/,
				  Pt3D&           ref   )
{
   TruncatedPyramid::localCorners( lc, pv, ref ) ;
}

void
EcalEndcapGeometry::newCell( const GlobalPoint& f1 ,
			     const GlobalPoint& f2 ,
			     const GlobalPoint& f3 ,
			     const CCGFloat*    parm ,
			     const DetId&       detId   ) 
{
   const unsigned int cellIndex ( EEDetId( detId ).denseIndex() ) ;
   m_cellVec[ cellIndex ] =
      TruncatedPyramid( cornersMgr(), f1, f2, f3, parm ) ;
   if( fabs(f1.z())<300. ) std::cout<<"***For id="<<EEDetId(detId)<<" bad z:"<<f1<<std::endl;
   m_validIds.push_back( detId ) ;
}

CCGFloat 
EcalEndcapGeometry::avgAbsZFrontFaceCenter() const
{
   if( 0 > m_avgZ )
   {
      CCGFloat sum ( 0 ) ;
      for( unsigned int i ( 0 ) ; i != m_cellVec.size()/2 ; ++i )
      {
	 const CaloCellGeometry* cell ( cellGeomPtr(i) ) ;
	 if( 0 != cell )
	 {
	    sum += fabs( cell->getPosition().z() ) ;
	 }
      }
      m_avgZ = 2*sum/m_cellVec.size() ;
   }
   return m_avgZ ;
}

const CaloCellGeometry* 
EcalEndcapGeometry::cellGeomPtr( uint32_t index ) const
{
   const CaloCellGeometry* cell ( &m_cellVec[ index ] ) ;
   return ( m_cellVec.size() < index ||
	    0 == cell->param() ? 0 : cell ) ;
}
