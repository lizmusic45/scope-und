
import PySimpleGUI as sg
import matplotlib
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
import serial
from serial.tools import list_ports
from io import StringIO


# set the theme for the screen/window
sg.theme("DarkTanBlue")
# define layout

PORT_list = list_ports.comports()
PORTlen = len(PORT_list)

PORTname_list = [PORT_list[i][1] for i in range(PORTlen)]
PORTname_list.sort()
PORTname_list.insert(0,'')

VertList = ('10mV', '100mV', '1V')
VertListMultiplier = (2, 1, 0) #in volts, so 10mV = 10^-2, 100mV = 10^-1, 1V = 10^0
TrigList = ('mV', 'V')
TrigListMultiplier = (3, 0) #10^-3, 10^0
HorizList = ('10us', '100us', '1ms', '10ms', '100ms')
HorizListMultiplier = (1, 2, 3, 4, 5) #in us, so 10us = 10^1, 100us = 10^2....etc
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


serialportselect=[sg.Text('Serial'), sg.Combo(values=PORTname_list, size=(30,1), readonly=True,
                                              default_value='', enable_events=True, key='Serial')]

options = [
            [sg.Frame('Channels',
                [
                    [sg.Checkbox('Ch1', default=True, key='Ch1'),
                     sg.Checkbox('Ch2', key='Ch2'),
                     sg.Checkbox('Ch3', key='Ch3'),
                     sg.Checkbox('Ch4', key='Ch4')]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Vertical',
                [
                    [sg.Slider(range=(1, 5), orientation='v', size=(5, 28), default_value=1, resolution=2,
                               tick_interval=2, disable_number_display=True, pad=(11, 0), key='Vert1'),
                     sg.Slider(range=(1, 5), orientation='v', size=(5, 28), default_value=1, resolution=2,
                               tick_interval=2, disable_number_display=True, pad=(11, 0), key='Vert2'),
                     sg.Slider(range=(1, 5), orientation='v', size=(5, 28), default_value=1, resolution=2,
                               tick_interval=2, disable_number_display=True, pad=(11, 0), key='Vert3'),
                     sg.Slider(range=(1, 5), orientation='v', size=(5, 28), default_value=1, resolution=2,
                               tick_interval=2, disable_number_display=True, pad=(11, 0), key='Vert4')],

                    [sg.Spin(values = VertList, size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert1Scale'),
                     sg.Spin(values = VertList, size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert2Scale'),
                     sg.Spin(values = VertList, size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert3Scale'),
                     sg.Spin(values = VertList, size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Vert4Scale')]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Triggers',
                [
                    [sg.Radio('Ch1', 'rd_triggers', default=True, key='Trig1'),
                     sg.Radio('Ch2', 'rd_triggers', key='Trig2'),
                     sg.Radio('Ch3', 'rd_triggers', key='Trig3'),
                     sg.Radio('Ch4', 'rd_triggers', key='Trig4')],

                    [sg.Text('Type:'), sg.Radio('Free', 'rd_type', key='Free'),
                     sg.Radio('Rise', 'rd_type', default=True, key='Rise'),
                     sg.Radio('Fall', 'rd_type', key='Fall')],

                    [sg.Text('Level:'), sg.Input(size=(4,18), default_text='10',
                                                 enable_events=True, key='TrigLevel'),
                     sg.Spin(values=TrigList, initial_value='mV', size=(3,18), key='TrigLevelScale')]
                ],
            title_color='yellow',
            border_width = 10)],

            [sg.Frame('Horizontal',
                [
                    [sg.Slider(range=(2, 8), orientation='h', size=(15, 25), default_value=2, resolution=2,
                               tick_interval=2, disable_number_display=True, key='Horiz'),
                     sg.Spin(values=HorizList, size=(12, 20), initial_value='1ms',
                             font=('Helvetica', 8), key='HorizScale')]
                ],
            title_color='yellow',
            border_width = 10)]
          ]

#---------------Data that Alex will supply with his code.  this is just a data set for an example ------------------
ch1fns = ['Ch1', '4.0 V', '1 Hz', '2.83 V', '-2.0 V', '2.0 V']
ch2fns = ['Ch2', '-', '-', '-', '-', '-']
ch3fns = ['Ch3', '-', '-', '-', '-', '-']
ch4fns = ['Ch4', '-', '-', '-', '-', '-']

#------------------the table headings-----------------------------------------------------
headings = ['CH', 'Pk-to-Pk', 'Freq', 'RMS', 'Min', 'Max']

fns = [
        sg.Button('Submit', font=('Times New Roman', 12)),
        sg.Button('Close', font=('Times New Roman', 12)),
        sg.Button('Acquire', font=('Times New Roman', 12)),

        sg.Frame('Functions',
            [
                [sg.Table(values=([ch1fns, ch2fns, ch3fns, ch4fns]), headings=headings,
                          auto_size_columns=False, col_widths=[8,8,8,8,8,8], num_rows=4,
                          row_colors=[(0,'blue'), (1,'orange'), (2,'green'), (3,'red')],
                          hide_vertical_scroll=True, justification='center')
                ]
            ],
        pad=((200,0),(0,0)),
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

# ------------------------------- Beginning of GUI CODE -------------------------------

# Create layout with two columns using pre-created frames
layout = [[sg.Column(choices), (sg.Canvas(key='-CANVAS-'))]],fns

# create the form and show it without the plot
window = sg.Window('Demo Application - 4 Channel Oscilloscope', layout, finalize=True) #location=(100, 25)

# add the plot to the window
#fig_canvas_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)


while True:
    event, values = window.read()  # Read  values entered by user
    settings_list=values

    if event == sg.WIN_CLOSED:  # If window is closed by user terminate While Loop
        break
    if event == 'Serial':

        for j in range(len(PORT_list)):
            if PORT_list[j][1] == settings_list['Serial']:
                PORT_index = j

        PORT_name=PORT_list[PORT_index][0]

        try:
            arduino = serial.Serial(PORT_name, baudrate=9600, timeout=0.1)

        except serial.SerialException:
            PORTname_list.remove(settings_list['Serial'])
            window['Serial'].update(value='', values=PORTname_list)
            sg.Popup('That port is busy.  You must select a different port', background_color='pink',
                     relative_location=(-125,0), keep_on_top=True, text_color='black')

    if event == 'TrigLevel' and values['TrigLevel'] and (values['TrigLevel'][-1] not in ('0123456789.')
                                                         or len(values['TrigLevel']) > 4):
        window['TrigLevel'].update(values['TrigLevel'][:-1])

    if event == 'Acquire':

        if settings_list['Serial']=='':
            sg.Popup('You must select a port in the Serial list', background_color='pink',
                     relative_location=(-125,0), keep_on_top=True, text_color='black')

        else:
            arduino.write(b'a')

            arduino.read_until(b'a')

            data = arduino.read_until(b'e')[:-1]
            print(data)

            strData = data.decode()

            strDataFile = StringIO('Ch1,'+'Ch2,' + 'Ch3,' + 'Ch4,' + 't\n' + strData)

            df = pd.read_csv(strDataFile, sep=',', lineterminator='\n')

            print(df)

            fig = matplotlib.figure.Figure(figsize=(5, 4), dpi=100)
            fig.add_subplot(111).plot(df["t"], df["Ch1"], df["t"], df["Ch2"], df["t"], df["Ch3"], df["t"], df["Ch4"])   
            fig_canvas_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)
            
            #-------Alex your pandas functions can go here--------#
            #-------you will have to figure out how to update the table I made earlier in the code-----#
            #-------the table code is lines 116-137---------------#


    if event == 'Submit':

        if settings_list['Serial']=='':
            sg.Popup('You must select a port in the Serial list', background_color='pink',
                     relative_location=(-125,0), keep_on_top=True, text_color='black')

        else:

            #---formatting the data to send over the serial---------#
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

            TrigType = [settings_list['Free'], settings_list['Rise'], settings_list['Fall']].index(1)

            TrigLevelMult = TrigListMultiplier[TrigList.index(settings_list['TrigLevelScale'])]

            if settings_list['Free']==1:
                TrigLevel = '0.00'
            else:
                if (len(settings_list['TrigLevel']) < 4):
                    if ('.' not in settings_list['TrigLevel']):
                        TrigLevel = settings_list['TrigLevel'] + '.'
                    else:
                        TrigLevel = settings_list['TrigLevel']
                    while len(TrigLevel) < 4:
                        TrigLevel = TrigLevel + '0'
                else:
                    TrigLevel = settings_list['TrigLevel']

            TrigLevelSet = [TrigLevel[k] for k in range(len(TrigLevel))]

            Settings = V + VMult + [H] + [HMult] + [TrigCh] + [TrigType] + [TrigLevelMult] + TrigLevelSet

            SendSettings = np.array(Settings)
            SendSettingsbyte = SendSettings.astype(bytes)

            #print(ChanStr)
            #print(ChanByte)
            #print(SendSettingsbyte)

            arduino.write(b's')

            arduino.read_until(b's')

            arduino.write(ChanByte)

            for k in range(len(SendSettingsbyte)):
                arduino.write(SendSettingsbyte[k])

            arduino.write(b'e')


    if event == 'Close':
        break
# Close Window
window.close()

