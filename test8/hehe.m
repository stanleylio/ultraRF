clc
close all
clear all

d0 = import0m('0m.txt');
d0_5 = import0_5m('0.5m.txt');
d1 = import1m('1m.txt');
d1_5 = import1_5m('1.5m.txt');

chop = min([numel(d0),numel(d0_5),numel(d1),numel(d1_5)]);
d0 = d0(1:chop);
d0_5 = d0_5(1:chop);
d1 = d1(1:chop);
d1_5 = d1_5(1:chop);

D = [d0 d0_5 d1 d1_5];
size(D)
D = medfilt2(D,[5,1]);

figure
plot(D)
title('')
xlabel('Experiment Index')
ylabel('ADC Sample Index (RF pulse to Ultrasound pulse Delay)')
legend('0m','0.5m','1m','1.5m','location','best')

saveas(gcf,'ranging.png')
