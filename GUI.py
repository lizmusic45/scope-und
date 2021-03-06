import PySimpleGUI as sg
import matplotlib as mpl
formatterV = mpl.ticker.EngFormatter(places=2, unit='V')
formatterF = mpl.ticker.EngFormatter(places=2, unit='Hz')
formatterS = mpl.ticker.EngFormatter(places=0, unit='sps')
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial
from serial.tools import list_ports
from io import StringIO
import math as math
import warnings
warnings.filterwarnings("error")
import scipy as sy
import scipy.fftpack


# set the theme for the screen/window
sg.theme("DarkTanBlue")
# define layout


VertList = ('10mV', '100mV', '1V')
VertListMultiplier = (2, 1, 0) #in volts, so 10mV = 10^-2, 100mV = 10^-1, 1V = 10^0
HorizList = ('100us', '1ms', '10ms', '100ms')
HorizListMultiplier = (2, 3, 4, 5) #in us, so 100us = 10^2....etc
chDict = {
    '0b00000000': b'A',
    '0b00000010': b'B',
    '0b00001000': b'C',
    '0b00001010': b'D',
    '0b00100000': b'E',
    '0b00100010': b'F',
    '0b00101000': b'G',
    '0b00101010': b'H',
    '0b10000000': b'I',
    '0b10000010': b'J',
    '0b10001000': b'K',
    '0b10001010': b'L',
    '0b10100000': b'M',
    '0b10100010': b'N',
    '0b10101000': b'O',
    '0b10101010': b'P'
}

firstOpen = 0
sendDefaultSettings = 0
beginSerial = 0
PORT_list = []
PORTname_list = ['']


serialportselect=[sg.Text('Serial'), sg.Combo(values=PORTname_list, size=(30,1), readonly=True,
                                              default_value='', enable_events=True, key='Serial')]
                  #sg.Button('Restart', font=('Times New Roman', 12))]

options = [
            [sg.Frame('Channels',
                [
                    [sg.Checkbox('Ch1', default=True, key='Ch1', enable_events=True),
                     sg.Checkbox('Ch2', key='Ch2', enable_events=True),
                     sg.Checkbox('Ch3', key='Ch3', enable_events=True),
                     sg.Checkbox('Ch4', key='Ch4', enable_events=True)]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Vertical',
                [
                    [sg.Slider(range=(1, 4), orientation='v', size=(5, 28), default_value=4, resolution=1,
                               tick_interval=1, disable_number_display=True, pad=(11, 0), key='Vert1',
                               enable_events=True),
                     sg.Slider(range=(1, 4), orientation='v', size=(5, 28), default_value=4, resolution=1,
                               tick_interval=1, disable_number_display=True, pad=(11, 0), key='Vert2',
                               enable_events=True),
                     sg.Slider(range=(1, 4), orientation='v', size=(5, 28), default_value=4, resolution=1,
                               tick_interval=1, disable_number_display=True, pad=(11, 0), key='Vert3',
                               enable_events=True),
                     sg.Slider(range=(1, 4), orientation='v', size=(5, 28), default_value=4, resolution=1,
                               tick_interval=1, disable_number_display=True, pad=(11, 0), key='Vert4',
                               enable_events=True)],

                    [sg.Spin(values=VertList, size=(6, 18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert1Scale', enable_events=True),
                     sg.Spin(values=VertList, size=(6, 18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert2Scale', enable_events=True),
                     sg.Spin(values=VertList, size=(6, 18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert3Scale', enable_events=True),
                     sg.Spin(values=VertList, size=(6, 18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert4Scale', enable_events=True)]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Triggers',
                [
                    [sg.Radio('Ch1', group_id='rd_triggers', default=True, key='Trig1'),
                     sg.Radio('Ch2', group_id='rd_triggers', key='Trig2'),
                     sg.Radio('Ch3', group_id='rd_triggers', key='Trig3'),
                     sg.Radio('Ch4', group_id='rd_triggers', key='Trig4')],

                    [sg.Text('Type:'),
                     sg.Radio('Free', group_id='rd_type', default=True, key='Free'),
                     sg.Radio('Rise', group_id='rd_type', key='Rise'),
                     sg.Radio('Fall', group_id='rd_type', key='Fall')],

                    [sg.Text('        '),
                     sg.Radio('Higher', group_id='rd_type', key='Higher'),
                     sg.Radio('Lower', group_id='rd_type', key='Lower')],

                    [sg.Text('Level:'), sg.Input(size=(6, 18), default_text='0.10',
                                                 enable_events=True, key='TrigLevel'),
                     sg.Text('V')]
                ],
            title_color='yellow',
            border_width = 10)],

            [sg.Frame('Horizontal',
                [
                    [sg.Slider(range=(1, 9), orientation='h', size=(15, 25), default_value=5, resolution=2,
                               tick_interval=2, disable_number_display=True, key='Horiz'),
                     sg.Spin(values=HorizList, size=(12, 20), initial_value='1ms',
                             font=('Helvetica', 8), key='HorizScale', enable_events=True)]
                ],
            title_color='yellow',
            border_width=10)]
          ]

#---------------Data that Alex will supply with his code.  this is just a data set for an example ------------------
ch1fns = ['Ch1', '4.0 V', '1 Hz', '2.83 V', '-2.0 V', '2.0 V']
ch2fns = ['Ch2', '-', '-', '-', '-', '-']
ch3fns = ['Ch3', '-', '-', '-', '-', '-']
ch4fns = ['Ch4', '-', '-', '-', '-', '-']

#------------------the table headings-----------------------------------------------------
headings = ['CH', 'Pk-to-Pk', 'Freq', 'RMS', 'Min', 'Max']

hold = [sg.Checkbox('Hold Screen', default=False, key='Hold', disabled=True)]
sampleText = [sg.Text('Sample Rate:')]
sample = [sg.Text('20 ksps', key='Sample Rate', pad=((20,0), (0,0)))]

fns = [
        sg.Button('Submit', font=('Times New Roman', 12), pad=((70,0), (0,0))),
        sg.Checkbox('Acquire', default=False, key='Acquire', enable_events=True),
        #sg.Button('Close', font=('Times New Roman', 12)),

        sg.Frame('Functions',
            [
                [sg.Table(values=([ch1fns, ch2fns, ch3fns, ch4fns]), headings=headings,
                          auto_size_columns=False, col_widths=[14, 14, 14, 14, 14, 14], num_rows=4,
                          row_colors=[(0, 'dodger blue', '#242834'), (1, 'red', '#242834'),
                                      (2, 'green2', '#242834'), (3, 'dark orange', '#242834')],
                          hide_vertical_scroll=True, justification='center', key='Functions'),
                sg.Column([hold, sampleText, sample])
                ]
            ],
        pad=((95, 0), (0, 0)),
        title_color='yellow',
        border_width=10),

      ]

choices = [serialportselect, [sg.Frame('Oscilloscope 4Channels', layout=options)]]



# ------------------------------- Beginning of Matplotlib helper code -----------------------

def draw_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

def delete_figure_agg(figure_agg):
    figure_agg.get_tk_widget().forget()

# ------------------------------- Beginning of GUI CODE -------------------------------

# Create layout with two columns using pre-created frames
layout = [[sg.Column(choices), (sg.Canvas(key='-CANVAS-'))]], fns

# create the form and show it without the plot
window = sg.Window('Demo Application - 4 Channel Oscilloscope', layout, finalize=True)

# Generating time data using arange function from numpy
time1 = np.arange(-3*np.pi, 3*np.pi, 0.01)

# Finding amplitude at each time
amplitude = np.sin(time1)

# Plotting time vs amplitude using plot function from pyplot
plt.figure(1, figsize=(9, 5), dpi=100, facecolor='#242834', frameon=False, constrained_layout=True)
ax = plt.axes()
#ax.set_facecolor(color='#242834')
ax.set_yticklabels([])
ax.set_xticklabels([])
plt.grid()
plt.plot(time1, amplitude, color='blue')
fig = plt.gcf()


figure_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)

while True:

    while firstOpen == 0:
        sg.Popup('Plug in the Teensy to a USB port and open the Arduino IDE',
                 background_color='pink', relative_location=(-125, 0), keep_on_top=True, text_color='black')
        sg.Popup('In the Tools menu select Board: "Teensy 4.0" and USB Type: "Dual Serial"',
                 background_color='pink', relative_location=(-125, 0), keep_on_top=True, text_color='black')
        firstOpen = 1

    while PORTname_list == ['']:

        PORT_list = list_ports.comports()
        PORTlen = len(PORT_list)

        PORTname_list = [PORT_list[i][1] for i in range(PORTlen)]
        PORTname_list.sort()
        PORTname_list.insert(0, '')
        window['Serial'].update(values=PORTname_list)

        if PORT_list == []:

            sg.Popup('Plug in the Teensy to a USB port.', background_color='pink',
                     relative_location=(-125, 0), keep_on_top=True, text_color='black')

            PORT_list = list_ports.comports()
            PORTlen = len(PORT_list)

            PORTname_list = [PORT_list[i][1] for i in range(PORTlen)]
            PORTname_list.sort()
            PORTname_list.insert(0, '')
            window['Serial'].update(values=PORTname_list)

    if beginSerial == 0:
        if window['Serial'] != '':
            window['Serial'].update('')

        sg.Popup('Select a port in the Serial list', background_color='pink',
                 relative_location=(-125, 0), non_blocking=True, keep_on_top=True,
                 text_color='black')

    event, values = window.read()  # Read  values entered by user
    settings_list=values


    if event == sg.WIN_CLOSED:  # If window is closed by user terminate While Loop
        break

    #if event == 'Close':
        #break

    if figure_agg:
        delete_figure_agg(figure_agg)
        plt.figure(1, clear=True)

    #if event == 'Restart':

        #try:
            #arduino.close()
        #except:
            #continue

        #beginSerial = 0
        #sendDefaultSettings = 0
        #Portname_list = ['']
        #window['Serial'].update(value='', values=PORTname_list, disabled=False)

    if event == 'Serial':

        if settings_list['Serial'] != '':
            try:
                for j in range(len(PORT_list)):
                    if PORT_list[j][1] == settings_list['Serial']:
                        PORT_index = j

                PORT_name=PORT_list[PORT_index][0]

            except NameError:
                beginSerial = 0
                sendDefaultSettings = 0
                PORTname_list = ['']
                window['Serial'].update(value='', values=PORTname_list)

            if beginSerial == 0:
                try:
                    arduino = serial.Serial(PORT_name, baudrate=9600) #, timeout=0.1)
                    beginSerial = 1
                    sendDefaultSettings = 1
                    window['Serial'].update(disabled=True)

                except NameError:
                    beginSerial = 0
                    sendDefaultSettings = 0
                    PORTname_list = ['']
                    window['Serial'].update(value='', values=PORTname_list)

                except serial.SerialException:
                    PORTname_list.remove(settings_list['Serial'])
                    window['Serial'].update(value='', values=PORTname_list)
                    sg.Popup('That port is for the Serial Plotter.', background_color='pink',
                             relative_location=(-125,0), keep_on_top=True, text_color='black')


    if event == 'TrigLevel' and values['TrigLevel']:

        if len(values['TrigLevel']) == 1:
            if values['TrigLevel'] not in ('0123456789.-'):
                window['TrigLevel'].update(values['TrigLevel'][:-1])

        if len(values['TrigLevel']) == 2:
            if values['TrigLevel'][0] == '-':
                if values['TrigLevel'][-1] not in ('0123456789.'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][0] == '0':
                if values['TrigLevel'][-1] not in ('.'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][0] == '.':
                if values['TrigLevel'][-1] not in ('0123456789'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][0] == '2':
                if values['TrigLevel'][-1] not in ('0.'):
                    window['TrigLevel'].update('20')
                    sg.Popup('The trigger level cannot be greater than +20 V', background_color='pink',
                             relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][0] in ('3456789'):
                if values['TrigLevel'][-1] not in ('.'):
                    window['TrigLevel'].update('20')
                    sg.Popup('The trigger level cannot be greater than +20 V', background_color='pink',
                             relative_location=(-125, 0), keep_on_top=True, text_color='black')

        if len(values['TrigLevel']) == 3:
            if values['TrigLevel'][0] == '.' or values['TrigLevel'][1] == '.':
                if values['TrigLevel'][-1] not in ('0123456789'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][0] == '-' and values['TrigLevel'][1] == '0':
                if values['TrigLevel'][-1] not in ('.'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][0] == '-' and values['TrigLevel'][1] == '2':
                if values['TrigLevel'][-1] not in ('0.'):
                    window['TrigLevel'].update('-20')
                    sg.Popup('The trigger level cannot be less than -20 V', background_color='pink',
                             relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][0] == '-' and values['TrigLevel'][1] in ('3456789'):
                if values['TrigLevel'][-1] not in ('.'):
                    window['TrigLevel'].update('-20')
                    sg.Popup('The trigger level cannot be less than -20 V', background_color='pink',
                             relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][0] == '2' and values['TrigLevel'][1] == '0':
                window['TrigLevel'].update(values['TrigLevel'][:-1])
                sg.Popup('The trigger level cannot be greater than +20 V', background_color='pink',
                         relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][-1] not in ('0123456789.'):
                window['TrigLevel'].update(values['TrigLevel'][:-1])

        if len(values['TrigLevel']) == 4:
            if values['TrigLevel'][0] == '-' and values['TrigLevel'][1] == '2' and values['TrigLevel'][2] == '0':
                window['TrigLevel'].update(values['TrigLevel'][:-1])
                sg.Popup('The trigger level cannot be less than -20 V', background_color='pink',
                         relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][0] == '.':
                window['TrigLevel'].update(values['TrigLevel'][:-1])
                sg.Popup('The trigger level precision is to the tenths place only', background_color='pink',
                         relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][1] == '.' or values['TrigLevel'][2] == '.':
                if values['TrigLevel'][-1] not in ('0123456789'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][-1] not in ('0123456789.'):
                window['TrigLevel'].update(values['TrigLevel'][:-1])

        if len(values['TrigLevel']) == 5:
            if values['TrigLevel'][1] == '.':
                window['TrigLevel'].update(values['TrigLevel'][:-1])
                sg.Popup('The trigger level precision is to the tenths place only', background_color='pink',
                         relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][2] == '.':
                if values['TrigLevel'][-1] not in ('0123456789'):
                    window['TrigLevel'].update(values['TrigLevel'][:-1])
            elif values['TrigLevel'][-1] not in ('0123456789'):
                window['TrigLevel'].update(values['TrigLevel'][:-1])

        if len(values['TrigLevel']) == 6:
            if values['TrigLevel'][2] == '.':
                window['TrigLevel'].update(values['TrigLevel'][:-1])
                sg.Popup('The trigger level precision is to the tenths place only', background_color='pink',
                         relative_location=(-125, 0), keep_on_top=True, text_color='black')
            elif values['TrigLevel'][3] == '.':
                if values['TrigLevel'][-1] not in ('0123456789'):
                    window['TrigLevel'].update(values['TrigLevel'][-1])
            elif values['TrigLevel'][-1] not in ('0123456789'):
                window['TrigLevel'].update(values['TrigLevel'][:-1])

        if len(values['TrigLevel']) > 6:
            window['TrigLevel'].update(values['TrigLevel'][:-1])
            sg.Popup('The trigger level precision is to the tenths place only', background_color='pink',
                     relative_location=(-125, 0), keep_on_top=True, text_color='black')

    if event == 'HorizScale':
        if values['HorizScale'] == '100us':
            window['Horiz'].update(range=(5, 9))
        else:
            window['Horiz'].update(range=(1, 9))

    if event == 'Submit' or sendDefaultSettings == 1:

        #---formatting the data to send to the Teensy---------#

        ChanStr = '0b'+str(1*settings_list['Ch1'])+'0'+str(1*settings_list['Ch2'])+'0'+str(1*settings_list['Ch3'])+'0'+str(1*settings_list['Ch4'])+'0'

        ChanByte = chDict[ChanStr]

        V = [int(settings_list['Vert1']*settings_list['Ch1']), int(settings_list['Vert2']*settings_list['Ch2']),
                 int(settings_list['Vert3']*settings_list['Ch3']), int(settings_list['Vert4']*settings_list['Ch4'])]

        VMult = [VertListMultiplier[VertList.index(settings_list['Vert1Scale'])],
                     VertListMultiplier[VertList.index(settings_list['Vert2Scale'])],
                     VertListMultiplier[VertList.index(settings_list['Vert3Scale'])],
                     VertListMultiplier[VertList.index(settings_list['Vert4Scale'])]]

        H = int(settings_list['Horiz'])

        HMult = HorizListMultiplier[HorizList.index(settings_list['HorizScale'])]

        TrigCh = [settings_list['Trig1'], settings_list['Trig2'], settings_list['Trig3'],
                      settings_list['Trig4']].index(1)

        TrigType = [settings_list['Free'], settings_list['Rise'], settings_list['Fall'],
                        settings_list['Higher'], settings_list['Lower']].index(1)

        if settings_list['Free']==1:
            TrigLevel = '0000'
            TrigLevelSign = '0'
        else:
            if settings_list['TrigLevel'][0] == '-':
                TrigLevelSign = '1'
                TrigLevel = settings_list['TrigLevel'][1:]
            else:
                TrigLevelSign = '0'
                TrigLevel = settings_list['TrigLevel']
            if '.' not in TrigLevel:
                TrigLevel = TrigLevel + '.'
            else:
                TrigLevel = TrigLevel
            if TrigLevel[0] == '.':
                TrigLevel = '0' + '0' + TrigLevel[1:]
            elif TrigLevel[1] == '.':
                TrigLevel = '0' + TrigLevel[0] + TrigLevel[2:]
            elif TrigLevel[2] == '.':
                TrigLevel = TrigLevel[0] + TrigLevel[1] + TrigLevel[3:]
            else:
                TrigLevel = TrigLevel

            while len(TrigLevel) < 4:
                TrigLevel = TrigLevel + '0'

        A = int(settings_list['Acquire']*1)

        Settings = V + VMult + [H] + [HMult] + [TrigCh] + [TrigType] + [TrigLevelSign] + [TrigLevel] + [A]

        SendSettings = np.array(Settings)
        SendSettingsbyte = SendSettings.astype(bytes)

            #----------writing the settings to the Teensy-------------------#

        try:
            arduino.write(b's')

            arduino.read_until(b's')

            arduino.write(ChanByte)

            for k in range(len(SendSettingsbyte)):
                arduino.write(SendSettingsbyte[k])

            arduino.write(b'e')

            # ------if Acquire is selected, get the data from the Teensy-------------#

            if settings_list['Acquire'] == True:
                    # ----clear plot?-----#

                arduino.write(b'a')

                arduino.read_until(b'a')

                data1 = arduino.read_until(b'e')[:-1]
                data2 = arduino.read_until(b'f')[:-1]
                data3 = arduino.read_until(b'g')[:-1]
                data4 = arduino.read_until(b'h')[:-1]
                data5 = arduino.read_until(b'i')[:-1]

                data = data1 + data2 + data3 + data4 + data5

                strData = data.decode()

                strDataFile = StringIO('Ch1,' + 'Ch2,' + 'Ch3,' + 'Ch4,' + 't\n' + strData)

                df = pd.read_csv(strDataFile, sep=',', lineterminator='\n')
                dfNoDC = df-df.mean()

                print(df)

                time2 = df['t'].values
                ch1v = df['Ch1'].values
                ch1vNoDC = dfNoDC['Ch1'].values
                ch2v = df['Ch2'].values
                ch2vNoDC = dfNoDC['Ch2'].values
                ch3v = df['Ch3'].values
                ch3vNoDC = dfNoDC['Ch3'].values
                ch4v = df['Ch4'].values
                ch4vNoDC = dfNoDC['Ch4'].values

                if values['Ch1']:
                    ch1vsqr = []
                    ch1vrms = 0
                    n = len(time2)
                    for i in ch1v:
                        ch1vsqr.append(i * i)
                    ch1vrms = formatterV(math.sqrt(sum(ch1vsqr) / n))
                    ch1vmin = formatterV(df['Ch1'].min())
                    ch1vmax = formatterV(df['Ch1'].max())
                    ch1vp_p = formatterV(df['Ch1'].max()-df['Ch1'].min())
                    FFT1 = sy.fftpack.fft(ch1vNoDC)
                    FFT1norm = 2/len(ch1vNoDC) * np.abs(FFT1)
                    freqs1 = sy.fftpack.fftfreq(len(ch1vNoDC), (time2[-1]/len(ch1vNoDC))) * 10**6
                    ch1f = formatterF(freqs1[FFT1norm.argmax()])
                    ch1fns[1] = ch1vp_p
                    ch1fns[2] = ch1f
                    ch1fns[3] = ch1vrms
                    ch1fns[4] = ch1vmin
                    ch1fns[5] = ch1vmax
                else:
                    ch1fns = ['Ch1', '-', '-', '-', '-', '-']
                if values['Ch2']:
                    ch2vsqr = []
                    ch2vrms = 0
                    n = len(time2)
                    for i in ch2v:
                        ch2vsqr.append(i * i)
                    ch2vrms = formatterV(math.sqrt(sum(ch2vsqr) / n))
                    ch2vmin = formatterV(df['Ch2'].min())
                    ch2vmax = formatterV(df['Ch2'].max())
                    ch2vp_p = formatterV(df['Ch2'].max() - df['Ch2'].min())
                    FFT2 = sy.fftpack.fft(ch2vNoDC)
                    FFT2norm = 2/len(ch2vNoDC) * np.abs(FFT2)
                    freqs2 = sy.fftpack.fftfreq(len(ch2vNoDC), (time2[-1]/len(ch2vNoDC))) * 10**6
                    if len(FFT2norm) == 0:
                        ch2f = 'DC'
                    else:
                        ch2f = formatterF(freqs2[FFT2norm.argmax()])
                    ch2fns[1] = ch2vp_p
                    ch2fns[2] = ch2f
                    ch2fns[3] = ch2vrms
                    ch2fns[4] = ch2vmin
                    ch2fns[5] = ch2vmax
                else:
                    ch2fns = ['Ch2', '-', '-', '-', '-', '-']
                if values['Ch3']:
                    ch3vsqr = []
                    ch3vrms = 0
                    n = len(time2)
                    for i in ch3v:
                        ch3vsqr.append(i * i)
                    ch3vrms = formatterV(math.sqrt(sum(ch3vsqr) / n))
                    ch3vmin = formatterV(df['Ch3'].min())
                    ch3vmax = formatterV(df['Ch3'].max())
                    ch3vp_p = formatterV(df['Ch3'].max() - df['Ch3'].min())
                    FFT3 = sy.fftpack.fft(ch3vNoDC)
                    FFT3norm = 2/len(ch3vNoDC) * np.abs(FFT3)
                    freqs3 = sy.fftpack.fftfreq(len(ch3vNoDC), (time2[-1]/len(ch3vNoDC))) * 10**6
                    ch3f = formatterF(freqs3[FFT3norm.argmax()])
                    ch3fns[1] = ch3vp_p
                    ch3fns[2] = ch3f
                    ch3fns[3] = ch3vrms
                    ch3fns[4] = ch3vmin
                    ch3fns[5] = ch3vmax
                else:
                    ch3fns = ['Ch3', '-', '-', '-', '-', '-']
                if values['Ch4']:
                    ch4vsqr = []
                    ch4vrms = 0
                    n = len(time2)
                    for i in ch4v:
                        ch4vsqr.append(i * i)
                    ch4vrms = formatterV(math.sqrt(sum(ch4vsqr) / n))
                    ch4vmin = formatterV(df['Ch4'].min())
                    ch4vmax = formatterV(df['Ch4'].max())
                    ch4vp_p = formatterV(df['Ch4'].max() - df['Ch4'].min())
                    FFT4 = sy.fftpack.fft(ch4vNoDC)
                    FFT4norm = 2/len(ch4vNoDC) * np.abs(FFT4)
                    freqs4 = sy.fftpack.fftfreq(len(ch4vNoDC), (time2[-1]/len(ch4vNoDC))) * 10**6
                    ch4f = formatterF(np.abs(freqs4[FFT4norm.argmax()]))
                    ch4fns[1] = ch4vp_p
                    ch4fns[2] = ch4f
                    ch4fns[3] = ch4vrms
                    ch4fns[4] = ch4vmin
                    ch4fns[5] = ch4vmax
                else:
                    ch4fns = ['Ch4', '-', '-', '-', '-', '-']
                window['Functions'].update(values=([ch1fns, ch2fns, ch3fns, ch4fns]),
                                           row_colors=[(0, 'dodger blue', '#242834'), (1, 'red', '#242834'),
                                                       (2, 'green2', '#242834'), (3, 'dark orange', '#242834')])

                sps = formatterS(1/((time2[-1]*10**(-6))/500))
                window['Sample Rate'].update(sps)

                plt.figure(1, figsize=(9, 5), dpi=100, facecolor='#242834')
                ax = plt.axes()
                #ax.set_facecolor(color='#242834')
                ax.set_yticklabels([])
                ax.set_xticklabels([])
                plt.axhline(4096 / 2, 0, 500, color='black')  # will eventually be 500
                plt.grid()
                plt.ylim(0, 4095)
                plt.xlim(0, 500)  # will eventually be 500
                plt.yticks(np.arange(0, 4095, 410))
                plt.xticks(np.arange(0, 500, 100))  # will eventually be 0, 500, 100
                plt.tick_params(grid_alpha=0.5)
                plt.plot(df.index, (((df["Ch1"]*4)/(V[0]*10**-VMult[0]))+20)*(4095-0)/(20+20)+0, 'blue',
                         df.index, (((df["Ch2"]*4)/(V[1]*10**-VMult[1]))+20)*(4095-0)/(20+20)+0, 'red',
                         df.index, (((df["Ch3"]*4)/(V[2]*10**-VMult[2]))+20)*(4095-0)/(20+20)+0, 'green',
                         df.index, (((df["Ch4"]*4)/(V[3]*10**-VMult[3]))+20)*(4095-0)/(20+20)+0, 'orange')

                fig = plt.gcf()

                figure_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)
                window['Hold'].update(True)

            sendDefaultSettings = 0

        except Exception as e:
            print(e)
            beginSerial = 0
            sendDefaultSettings = 0

    if event == 'Acquire' and settings_list['Acquire'] is False and settings_list['Hold'] is True:
        window['Hold'].update(False)

    if event == 'Vert1' and V[0] != 0 and settings_list['Hold'] is True or \
            event == 'Vert2' and V[1] != 0 and settings_list['Hold'] is True or \
            event == 'Vert3' and V[2] != 0 and settings_list['Hold'] is True or \
            event == 'Vert4' and V[3] != 0 and settings_list['Hold'] is True or \
            event == 'Vert1Scale' and V[0] != 0 and settings_list['Hold'] is True or \
            event == 'Vert2Scale' and V[1] != 0 and settings_list['Hold'] is True or \
            event == 'Vert3Scale' and V[2] != 0 and settings_list['Hold'] is True or \
            event == 'Vert4Scale' and V[3] != 0 and settings_list['Hold'] is True or \
            event == 'Ch1' and V[0] != 0 and settings_list['Hold'] is True or \
            event == 'Ch2' and V[1] != 0 and settings_list['Hold'] is True or \
            event == 'Ch3' and V[2] != 0 and settings_list['Hold'] is True or \
            event == 'Ch4' and V[3] != 0 and settings_list['Hold'] is True:

        plt.figure(1, figsize=(9, 5), dpi=100)
        ax = plt.axes()
        #ax.set_facecolor(color='#242834')
        ax.set_xticklabels([])
        ax.set_yticklabels([])
        plt.axhline(4096 / 2, 0, 500)  # will eventually be 500
        plt.grid()
        plt.ylim(0, 4095)
        plt.xlim(0, 500)  # will eventually be 500
        plt.yticks(np.arange(0, 4095, 410))
        plt.xticks(np.arange(0, 500, 100))  # will eventually be 0, 500, 100
        plt.tick_params(grid_alpha=0.5)
        plt.plot(df.index, (((df["Ch1"]*4*values['Ch1']) / (int(settings_list['Vert1']) * 10 ** \
                                          -VertListMultiplier[VertList.index(settings_list['Vert1Scale'])])) + 20) \
                 * (4095 - 0) / (20 + 20) + 0,
                 df.index, (((df["Ch2"]*4*values['Ch2']) / (int(settings_list['Vert2']) * 10 ** \
                                          -VertListMultiplier[VertList.index(settings_list['Vert2Scale'])])) + 20) \
                 * (4095 - 0) / (20 + 20) + 0,
                 df.index, (((df["Ch3"]*4*values['Ch3']) / (int(settings_list['Vert3']) * 10 ** \
                                          -VertListMultiplier[VertList.index(settings_list['Vert3Scale'])])) + 20) \
                 * (4095 - 0) / (20 + 20) + 0,
                 df.index, (((df["Ch4"]*4*values['Ch4']) / (int(settings_list['Vert4']) * 10 ** \
                                          -VertListMultiplier[VertList.index(settings_list['Vert4Scale'])])) + 20) \
                 * (4095 - 0) / (20 + 20) + 0)


        fig = plt.gcf()

        figure_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)
        window['Hold'].update(True)


# Close Window
window.close()

