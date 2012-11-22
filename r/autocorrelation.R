require('ggplot2');

multiplot <- function(..., plotlist=NULL, file, cols=1, layout=NULL) {
    require(grid)
    
    # Make a list from the ... arguments and plotlist
    plots <- c(list(...), plotlist)
    
    numPlots = length(plots)
    
    # If layout is NULL, then use 'cols' to determine layout
    if (is.null(layout)) {
        # Make the panel
        # ncol: Number of columns of plots
        # nrow: Number of rows needed, calculated from # of cols
        layout <- matrix(seq(1, cols * ceiling(numPlots/cols)),
        ncol = cols, nrow = ceiling(numPlots/cols))
    }
    
    if (numPlots==1) {
        print(plots[[1]])
        
    } else {
        # Set up the page
        grid.newpage()
        pushViewport(viewport(layout = grid.layout(nrow(layout), ncol(layout))))
        
        # Make each plot, in the correct location
        for (i in 1:numPlots) {
            # Get the i,j matrix positions of the regions that contain this subplot
            matchidx <- as.data.frame(which(layout == i, arr.ind = TRUE))
            
            print(plots[[i]], vp = viewport(layout.pos.row = matchidx$row,
            layout.pos.col = matchidx$col))
        }
    }
}

#long training sequence for computing correlation
long_training_seq <- c(-0.00512125+0.120325i, 0.15625+0i, -0.00512125-0.120325i, 0.0397497-0.111158i, 0.0968319+0.0827979i, 0.0211118+0.0278859i, 0.0598238-0.0877068i, -0.115131-0.0551805i, -0.038316-0.106171i, 0.0975413-0.0258883i, 0.0533377+0.00407633i, 0.00098898-0.115005i, -0.136805-0.0473798i, 0.0244759-0.0585318i, 0.0586688-0.014939i, -0.0224832+0.160657i, 0.119239-0.00409559i, 0.0625-0.0625i, 0.0369179+0.0983442i, -0.0572063+0.0392986i, -0.131263+0.0652272i, 0.0822183+0.0923566i, 0.0695568+0.014122i, -0.0603101+0.0812861i, -0.0564551-0.0218039i, -0.0350413-0.150888i, -0.121887-0.0165662i, -0.127324-0.0205014i, 0.0750737-0.0740404i, -0.00280594+0.0537743i, -0.0918876+0.115129i, 0.0917165+0.105872i, 0.0122846+0.0975996i, -0.15625+0i, 0.0122846-0.0975996i, 0.0917165-0.105872i, -0.0918876-0.115129i, -0.00280594-0.0537743i, 0.0750737+0.0740404i, -0.127324+0.0205014i, -0.121887+0.0165662i, -0.0350413+0.150888i, -0.0564551+0.0218039i, -0.0603101-0.0812861i, 0.0695568-0.014122i, 0.0822183-0.0923566i, -0.131263-0.0652272i, -0.0572063-0.0392986i, 0.0369179-0.0983442i, 0.0625+0.0625i, 0.119239+0.00409559i, -0.0224832-0.160657i, 0.0586688+0.014939i, 0.0244759+0.0585318i, -0.136805+0.0473798i, 0.00098898+0.115005i, 0.0533377-0.00407633i, 0.0975413+0.0258883i, -0.038316+0.106171i, -0.115131+0.0551805i, 0.0598238+0.0877068i, 0.0211118-0.0278859i, 0.0968319-0.0827979i, 0.0397497+0.111158i);

#function to compute signal power
compute.signal.power <- function(samples) {
    return (sum(Re(samples)^2 + Im(samples)^2));
}

#function to compute SNR
compute.signal.to.noise.ratio <- function (signal, noise) {
    return (10 * log10(compute.signal.power(signal) / compute.signal.power(noise)));
}

compute.auto.correlation <- function(samples, normalized_samples, scanWindowSize, start_i, stop_i) {

	L <- 32;

    #compute autocorrelation
    ac <- sum(samples[(stop_i+1-L):(stop_i+1)] * Conj(samples[(stop_i-16+1-L):(stop_i-16+1)]))
    #compute normalization value
	nsum <- sum(normalized_samples[c((start_i+16):stop_i)]);
    
    if (nsum < 0.00001) {
        return (0);
    }
    else {
        return (abs(Re(ac)) / nsum);
    }

}

compute.correlation <- function(samples, normalized_samples, window=64) {

    correlationSum <- sum(samples * Conj(long_training_seq))
    normalizedSum <- sum(normalized_samples);
    
    normFactor <- sqrt(normalizedSum);
    
    correlationSum <- correlationSum * normFactor;
    if (normalizedSum == 0) {
        return (0);
    }
    else {
        return (abs(correlationSum) / normalizedSum);
    }
    
}

detect.short.training.sequence <- function(signal) {
    
    correlationThr <- .4;
    begin <- 0;
    
    norm_samples <- Re(signal)^2 + Im(signal)^2;
    
    scanWindowSize <- 48;
    start_i <- 1;
    stop_i <- start_i + (scanWindowSize - 1);
    
    max_i <- length(signal) - scanWindowSize;
    autocorrelations <- c();
    while (start_i <= max_i & stop_i <= length(signal)) {
        autocorrelations <- c(autocorrelations, compute.auto.correlation(signal, norm_samples, scanWindowSize, start_i, stop_i));
                
        if ((begin == 0) & (autocorrelations[length(autocorrelations)] >= correlationThr)) {
            begin <- start_i;
        }
        
        start_i <- start_i + 1;
        stop_i <- stop_i + 1;
        
    }

    toplot <- data.frame(idx=1:length(autocorrelations), val=autocorrelations);
    
    plot <- ggplot(data=toplot, aes(x=idx, y=val)) + geom_line() + geom_vline(xintercept = begin, colour="red", linetype="longdash"); #+ xlim(c(0,250))  

    return (list(begin, plot));
    
}

detect.long.training.sequence <- function(signal, base_index, count) {

    norm_samples <- Re(signal)^2 + Im(signal)^2;
    
    start_i <- 1;
    stop_i <- start_i + 63;
    
    max_v1 <- -1;
    max_i1 <- 0;
    max_v2 <- -1;
    max_i2 <- 0;
    
    thr <- -1;
   
    correlations <- c();
    while (stop_i <= length(signal)) {

		corr <- compute.correlation(signal[start_i:stop_i], norm_samples[start_i:stop_i]);
    
        correlations <- c(correlations, corr);
        
        if (corr > max_v1) {
            max_v2 = max_v1;
            max_i2 = max_i1;
            max_v1 = corr;
            max_i1 = length(correlations);
        }
        else {
            if (corr > max_v2) {
                max_v2 = corr;
                max_i2 = length(correlations);
            }
        }
        
        start_i <- start_i + 1;
        stop_i <- stop_i + 1;
    }
   
   	ext <- c(rep(0, base_index-1), correlations, rep(0, count + 1 -  base_index - length(correlations)));
    
    toplot <- data.frame(idx=1:length(ext), val=ext);
    
    plot <- ggplot(data=toplot, aes(x=idx, y=val)) + geom_line()+ ylim(c(0,1))+ geom_vline(xintercept = max_i1 + base_index - 1, colour="red", linetype="longdash") + geom_vline(xintercept = max_i2 + base_index - 1, colour="red", linetype="longdash");
    
    #todo: substitute 0 with the two correlation peaks
    
    if (max_v1 > thr & max_v2 > thr) {
        peaks <- list(min(max_i1, max_i2), max(max_i1, max_i2));
    }
    else {
        peaks <- list(0,0);
    }
    
    return (list(peaks, plot));

}

initial_coarse_estimation <- function(short_samples) {
	return (Arg(sum(Conj(short_samples[1:(length(short_samples)-16)]) * short_samples[17:length(short_samples)])) / 16.0);  
}

initial_fine_estimation <- function(long_samples) {
	return (Arg(sum(Conj(long_samples[1:64]) * long_samples[65:128])) / 64.0);
}

apply.frequency.offset <- function(samples, phase_offset, direction) {
	return (samples * complex(real=cos(phase_offset*(1:length(samples))), imaginary=direction*sin(phase_offset*(1:length(samples)))));  
}

correct.long.training <- function(samples, coarse_est) {
	return(apply.frequency.offset(samples, coarse_est, -1));
}

get_channel_estimate <- function(samples) {

	begin_lt <- length(samples) - 160 + 1;

	end_st <- begin_lt - 1;
	num_st <- floor(end_st / 16);
	
	begin_st <- end_st - num_st * 16;
	
	coarse_est <- initial_coarse_estimation(samples[begin_st:end_st]); 

	print(paste("channel est: begin_st=", begin_st, "num_st=", num_st, "end_st=", end_st, "begin_lt=", begin_lt));
	print(paste("coarse_est=", coarse_est));

	corrected_lt <- correct.long.training(samples[(begin_lt+32):(begin_lt+160)], coarse_est);
 
	fine_est <- initial_fine_estimation(corrected_lt);
	
	print(paste("est freq offset:", (coarse_est + fine_est)));

	return (coarse_est + fine_est);
}

gg_color_hue <- function(n) {
    hues = seq(15, 375, length=n+1)
    hcl(h=hues, l=65, c=100)[1:n]
}







use.real.frame = T

if (use.real.frame) {
	#load data samples of a real ofdm frame captured using USRP
	load('../misc/captured_frame.Rdata');
	signal <- ex.sig #complex(real=frame[,2], imaginary=frame[,3]);
	cvalues <- signal
}else{
	#read frame from 802.11 example
	frame <- read.table('../misc/signal-2012.complex');

	#add 300 zero samples at beginning and at end (noise will be then added)
	extension <- 300;
	signal <- c(rep(complex(real=0), extension), signal);
	signal <- c(signal, rep(complex(real=0), extension));

	#standard deviation of AWGN
	sd <- .07;
	#set seed for generating different noises
	set.seed(12345);
	#now just generate noise
	noise <- complex(real=rnorm(n=length(signal), sd=sd),imaginary=rnorm(n=length(signal), sd=sd));

	freq_offset <- 4;

	#exp(j*2pi*Df*Ts*i)
	df <- freq_offset * (1e-6) * (5.9e9);
	phase <- 2 * pi * df * 1.0/10e6;
	print(paste("Frequency offsert =",(df/1000),"KHz, Phase offset = ", phase));

	freqsignal <- apply.frequency.offset(signal, phase, 1);

	#set of complex values to be analyzed: signal plus noise! Jo!
	cvalues <- freqsignal + noise;

	#print out SNR
	print(paste("Signal power:", compute.signal.power(signal), "Noise power:", compute.signal.power(noise), "SNR:", compute.signal.to.noise.ratio(signal, noise), "dB"));
}


#detect short training sequence
res_short_seq <- detect.short.training.sequence(cvalues);

start_short_sequence <- res_short_seq[[1]];
print(paste("Preamble start detected at sample", start_short_sequence));


#detect long training sequence
if (start_short_sequence+400 <= length(cvalues)) {
	res_long_seq <- detect.long.training.sequence(cvalues[(start_short_sequence+80):(start_short_sequence+400)], start_short_sequence+80, length(cvalues));
	start_long_sequence <- res_long_seq[[1]][[1]] - 32 + start_short_sequence + 80;
	print(paste("Long sequence starts at sample", start_long_sequence));
	detected_phase_offset <- get_channel_estimate(cvalues[start_short_sequence:(start_long_sequence+160)]);
} else {
	print(paste("Cannot detect long sequence start"));
	detected_phase_offset <- 0;
    start_long_sequence <- 0; 
}
#res_long_seq <- detect.long.training.sequence(cvalues);
#print(paste("Long traning sequence peaks at", res_long_seq[[1]][[1]], "and", res_long_seq[[1]][[2]]));


#coarse_offset <- initial_coarse_estimation(cvalues[start_short_sequence:(start_short_sequence+
corrected <- apply.frequency.offset(cvalues, detected_phase_offset, -1);

#plot complex time samples (actually only real part)

timesamples <- data.frame(idx=1:length(cvalues),real=Re(cvalues),imaginary=Im(cvalues));
corrected_timesamples <- data.frame(idx=1:length(corrected),real=Re(corrected),imaginary=Im(corrected));


colors <- gg_color_hue(4);

data_indexes <- (start_long_sequence + 160 + 16) + c(0:8) * 80;
data_ends <- data_indexes + 64;

datas_begin <- geom_vline(xintercept = data_indexes, colour=colors[3], linetype="longdash");
datas_end <- geom_vline(xintercept = data_ends, colour=colors[4], linetype="longdash");

ss_start = geom_vline(xintercept = start_short_sequence, colour=colors[1], linetype="longdash");
ls_start = geom_vline(xintercept = start_long_sequence, colour=colors[2], linetype="longdash");

plot_samples <- ggplot(data=timesamples, aes(x=idx, y=abs(real+1i*imaginary))) + geom_line(lw=2) +
				 ss_start + ls_start + datas_begin + datas_end;

plot_corrected_samples <- ggplot(data=corrected_timesamples, aes(x=idx, y=real)) + geom_line(lw=2) +
				 ss_start + ls_start + datas_begin + datas_end;

if (start_long_sequence != 0) {
	multiplot(res_short_seq[[2]], res_long_seq[[2]], plot_samples, plot_corrected_samples, cols=1);
} else {
	multiplot(res_short_seq[[2]], plot_samples, plot_corrected_samples, cols=1);
}
