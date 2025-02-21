## -*- texinfo -*-
## @deftypefn {Function File} {@var{outputImage}= } imageReconstruction_3D (@var{hradar}, @var{inputData}, @var{thetaBase}, @var{phyBase}, @var{rhoBase}, @var{algorithm})
## Create a 3D volume
## Input
## @itemize
## @item @var{hradar}: Structure containing stream acquisition parameters
## @item @var{hradar}.CoreFrequency: ADC sampling rate
## @item @var{hradar}.txCenterFrequency: Carrier frequency
## @item @var{hradar}.FixedTxToAntennaDelays: Array with transmitter antenna delay compensation (s)
## @item @var{hradar}.FixedRxToAntennaDelays: Array with receiver antenna delay compensation (s)
## @item @var{hradar}.TxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]
## @item @var{hradar}.RxAntPosition: Array with antenna position [x0, y0 z0; ... ;xn yn zn]
## @item @var{hradar}.TxRxCycle: Array containning the active antenna matrix for each acquisition cucle, the array is (2, NumAntennas, NumCycles)
## @item @var{inputData}: stream input (every row is a single stream)
## @item @var{thetaBase}: Reconstruction azimuth base
## @item @var{phyBase}: Reconstruction Zenith base
## @item @var{rhoBase}: Reconstruction distance base
## @item @var{algorithm}: selected reconstruction algorithm "DMAS", "DAS", "DMAS_SR", "DAS_SR"
## @end itemize
## Output
## @itemize
## @item @var{outputImage}: reconstructed image
## @end itemize
## @end deftypefn

% *************************************************
% ARIA Sensing srl 2024
% *************************************************

function outputImage = imageReconstruction_3D (hradar,inputData, thetaBase, phyBase, rhoBase ,algorithm)

  if (nargin < 6)
    print_usage();
  endif

  if (isempty(algorithm))
    algorithm = 'DAS';
  endif

  if (strcmp(algorithm, 'DAS'))
    DASSelected = 1;
    SREnabled = 0;
  elseif (strcmp(algorithm, 'DMAS'))
    DASSelected = 0;
    SREnabled = 0;
  elseif (strcmp(algorithm, 'DMAS_SR'))
    DASSelected = 0;
    SREnabled = 1;
  elseif  (strcmp(algorithm, 'DAS_SR'))
    DASSelected = 1;
    SREnabled = 1;
  else
    error("Invalid algorithm type")
  endif

  if (isempty(phyBase))
    phyBase = (-90:5:90) * pi/180;
  endif

  if (isempty(thetaBase))
    thetaBase = (0:5:360) * pi/180;
  endif

  #check coherence between provided data and active channels
  numStreams = size(inputData,1);
  antennaSeq = hradar.TxRxCycle;
  numCycles = size(antennaSeq,3);
  if (numStreams ~= numCycles)
    error("Number of streams must match the antenna sequence description")
  endif


  fadc = hradar.CoreFrequency;
  fc = hradar.txCenterFrequency;
  C0 = 299792458;
  numSamples = size(inputData,2);


  if (isempty(rhoBase))
    rhoBase = (0:1:(numSamples-1)) * (C0/(2*fadc));
  endif



  #create grids

  #create plane
  [Rr, Tt, Pp] = ndgrid(rhoBase, thetaBase, phyBase);

  XX = Rr .* cos(Tt) .* sin(Pp);
  YY = Rr .* sin(Tt) .* sin(Pp);
  ZZ = Rr .* cos(Pp);



  outputImage = zeros(size(Rr));


  #check cycles
  channelCombination = zeros(numCycles,2); #TxRx

  for N = 1:numCycles
    activeTx = sum(antennaSeq(1,:,N));
    activeRx = sum(antennaSeq(2,:,N));
    if ((activeTx ~= 1) || (activeRx ~= 1))
      error("Invalid Tx Rx scan combination")
    endif
    channelCombination(N,1) = find(antennaSeq(1,:,N) == 1);
    channelCombination(N,2) = find(antennaSeq(2,:,N) == 1);
  endfor

  TxAntDelay = hradar.FixedTxToAntennaDelays;
  RxAntDelay = hradar.FixedRxToAntennaDelays;



  inputData_pad = [inputData zeros(numStreams,1)]; #last index is used for out of bound condition

  #start cycling and reconstruction
  tbADC = (0:1:(numSamples))*1/fadc; #base is numsamples+1, last is for out of bound condition
  maxTfly = max(tbADC);
  minTfly = min(tbADC);

  #if (DASSelected == 0)
    remapStorage = zeros(numCycles, size(YY,1), size(YY,2), size(YY,3));
  #endif


  for N = 1:numCycles
    #get antenna
    TxI = channelCombination(N, 1);
    RxI = channelCombination(N, 2);
    curTx = hradar.TxAntPosition(TxI,:);
    curRx = hradar.RxAntPosition(RxI,:);
    curStream = inputData_pad(N,:);

    #compute delay remapping
    curTxXX = XX - curTx(1);
    curTxYY = YY - curTx(2);
    curTxZZ = ZZ - curTx(3);

    curRxXX = XX - curRx(1);
    curRxYY = YY - curRx(2);
    curRxZZ = ZZ - curRx(3);


    tflyMap = ((sqrt(curTxYY .^2+ curTxZZ.^2 + curTxXX.^2) + sqrt(curRxYY .^2+ curRxZZ.^2 + curRxXX.^2)) / C0);
    #carrier map
    #loMap = exp(1i*2*pi*(fc)*tflyMap); #create map for wave propagation
    #compensate internal delay
    tflyMap_comp = tflyMap+hradar.FixedTxToAntennaDelays(TxI) + hradar.FixedRxToAntennaDelays(RxI);
    loMap = exp(1i*2*pi*(fc)*tflyMap_comp); #create map for wave propagation

    oob_map = tflyMap_comp > maxTfly;
    oob_map = oob_map | (tflyMap_comp < minTfly);
    tflyMap_comp = (1-oob_map) .* tflyMap_comp + (oob_map).* tbADC(end); #map out of bound to zero

    proj = interp1(tbADC, curStream, tflyMap_comp) .* loMap;
    if (DASSelected)
      outputImage = outputImage + proj;
    #else
    endif
    remapStorage(N, :,:,:) =  proj;
  endfor

  if (DASSelected == 0)
    #peform DMAS
    for a = 1 : (numCycles-1)
      curA = squeeze(remapStorage(a,:,:,:));
      for k = (a+1):numCycles
        curB = squeeze(remapStorage(k,:,:,:));
        outputImage = outputImage + curA.*curB;
      endfor
    endfor
  else
    outputImage = (outputImage / (numCycles)); #normalization
  endif

  if (SREnabled == 1)
    CF = (abs(sum(remapStorage)).^2)./(numCycles * sum(abs(remapStorage).^2));
    CF = squeeze(CF(1,:,:,:));
    outputImage = outputImage .*CF;
  endif

endfunction
