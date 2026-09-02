CXX = g++
CC = gcc
CXXFLAGS = -std=c++17 -Iinclude -Iptutil -IptechDsp -I../audiopassthru/include -Iptutil/include -Wall -Wno-narrowing -Wno-write-strings -DDSPSOFT_TARGET -DPT_DSP_BUILD=2
CFLAGS = -Iinclude -Iptutil -IptechDsp -I../audiopassthru/include -Iptutil/include -Wall -Wno-narrowing -Wno-write-strings -DDSPSOFT_TARGET -DPT_DSP_BUILD=2

OBJS = \
	DfxDsp.o \
	DfxDspEq.o \
	DfxDspPreset.o \
	DfxDspPrivate.o \
	DfxDspRegistry.o \
	ptComSftDfx/Comsftwr.o \
	ptComSftDfx/ComsftwrCPP.o \
	ptechDsp/Aural/Aural032/Auralp32.o \
	ptechDsp/Aural/Aural0/Auralp.o \
	ptechDsp/Dly32/Dly832/dly8p32.o \
	ptechDsp/Dly8/Dly8p.o \
	ptechDsp/Lex/Lex16/Lex16.o \
	ptechDsp/Lex/Lex32/Lex32.o \
	ptechDsp/Maximizer/Maxi16/Maxi16.o \
	ptechDsp/Maximizer/Maxi32/Maxi32.o \
	ptechDsp/Peq/Peq832/peq8p32.o \
	ptechDsp/Peq/Peq8/peq8p.o \
	ptechDsp/Play/Play16/Play16.o \
	ptechDsp/Play/Play32/Play32.o \
	ptechDsp/wide/Wide16/Wide16.o \
	ptechDsp/wide/Wide32/Wide32.o \
	ptutil/COM/Com.o \
	ptutil/COM/Comeprom.o \
	ptutil/COM/Comget.o \
	ptutil/COM/ComMem.o \
	ptutil/COM/Compass.o \
	ptutil/COM/Comread.o \
	ptutil/COM/Comwave.o \
	ptutil/COM/Comwrite.o \
	ptutil/dfxp/dfxpComm.o \
	ptutil/dfxp/dfxpEq.o \
	ptutil/dfxp/dfxpGet.o \
	ptutil/dfxp/dfxpInit.o \
	ptutil/dfxp/dfxpProcess.o \
	ptutil/dfxp/dfxpProcessClear.o \
	ptutil/dfxp/dfxpProcessInt.o \
	ptutil/dfxp/dfxpProcessReal.o \
	ptutil/dfxp/dfxpQnt.o \
	ptutil/dfxp/dfxpQuit.o \
	ptutil/dfxp/dfxpRegistryStandard.o \
	ptutil/dfxp/dfxpSession.o \
	ptutil/dfxp/dfxpSet.o \
	ptutil/dfxp/dfxpSpectrum.o \
	ptutil/dfxp/dfxpUniversal.o \
	ptutil/dfxSharedUtil/dfxSharedUtil.o \
	ptutil/DspUtil/BinauralSync/BinauralSynGet.o \
	ptutil/DspUtil/BinauralSync/BinauralSynInit.o \
	ptutil/DspUtil/BinauralSync/BinauralSynProcess.o \
	ptutil/DspUtil/BinauralSync/BinauralSynSet.o \
	ptutil/DspUtil/GraphicEq/GraphicEqGet.o \
	ptutil/DspUtil/GraphicEq/GraphicEqInit.o \
	ptutil/DspUtil/GraphicEq/GraphicEqInitBands.o \
	ptutil/DspUtil/GraphicEq/GraphicEqInitSections.o \
	ptutil/DspUtil/GraphicEq/GraphicEqProcess.o \
	ptutil/DspUtil/GraphicEq/GraphicEqSet.o \
	ptutil/DspUtil/spectrum/spectrumGet.o \
	ptutil/DspUtil/spectrum/spectrumInit.o \
	ptutil/DspUtil/spectrum/spectrumMessageValues.o \
	ptutil/DspUtil/spectrum/spectrumProcess.o \
	ptutil/DspUtil/spectrum/spectrumReset.o \
	ptutil/DspUtil/spectrum/spectrumSet.o \
	ptutil/DspUtil/SurroundSyn/SurroundSynInit.o \
	ptutil/DspUtil/SurroundSyn/SurroundSynProcess.o \
	ptutil/Filt/Fil12But.o \
	ptutil/Filt/FiltbiqdSos.o \
	ptutil/Filt/FiltCalcBiqd.o \
	ptutil/Filt/FiltCalcFilterResponse.o \
	ptutil/Filt/Filtpoly.o \
	ptutil/Filt/FiltRun.o \
	ptutil/PRELST/Prelst.o \
	ptutil/PWAV/Pwav.o \
	ptutil/PWAV/PwavConvert.o \
	ptutil/PWAV/PwavIO.o \
	ptutil/PWAV/PwavUtil.o \
	ptutil/Qnt/Qnt.o \
	ptutil/Qnt/Qnt2But.o \
	ptutil/Qnt/QntitoBoostCut.o \
	ptutil/Qnt/Qntitol.o \
	ptutil/Qnt/Qntitor.o \
	ptutil/Qnt/Qntitor2.o \
	ptutil/Qnt/Qntrtoi.o \
	ptutil/Qnt/Qntrtol.o \
	ptutil/Qnt/Qntrtor.o \
	ptutil/realSample/realSampleForceLegalValues.o \
	ptutil/SOS/Sos.o \
	ptutil/SOS/SosGet.o \
	ptutil/SOS/SosProcess.o \
	ptutil/SOS/SosSet.o \
	ptutil/VALS/Vals.o \
	ptutil/VALS/Valscfg.o \
	ptutil/VALS/Valsfile.o \
	ptutil/VALS/Valsget.o \
	ptutil/VALS/Valsset.o \
	ptutil/VALS/Valswarp.o

libdfxdsp.a: $(OBJS)
	ar rcs $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) libdfxdsp.a
