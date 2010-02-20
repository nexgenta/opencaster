SUBDIRS = oc2sec sec2ts zpipe \
	tsfilter tsloop tsmodder tsnullfiller tsnullshaper \
	tspcrmeasure tspidmapper esaudio2pes esaudioinfo \
	esvideo2pes esvideoinfo pes2es pesaudio2ts \
	pesvideo2ts pesinfo tsstamp \
	ts2pes mpe2sec tscbrmuxer vbv tstdt i13942ts \
	tsvbr2cbr tsfixcc tsudpreceive tsudpsend dsmcc-receive \
	pesclock tspcrstamp tstcpreceive tstcpsend tstimeout \
	tstimedwrite tsinputswitch tsdoubleoutput pes2txt \
	tsoutputswitch totsectionrestamp tsccc txt2pes \
	pesdata2ts

subdirs:        $(SUBDIRS)
$(SUBDIRS):
	$(MAKE) -C $@
.PHONY:		subdirs $(SUBDIRS)

all: subdirs

install: all
	@for T in $(SUBDIRS); do make -C $$T $@; done

clean:
	@for T in $(SUBDIRS); do make -C $$T $@; done
