import PySimpleGUI as sg
import matplotlib
import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg



# set the theme for the screen/window
sg.theme("DarkTanBlue")
# define layout
options = [
            [sg.Frame('Channels',
                [
                    [sg.Checkbox('Ch1', key='Ch1'),
                     sg.Checkbox('Ch2', key='Ch2'),
                     sg.Checkbox('Ch3', key='Ch3'),
                     sg.Checkbox('Ch4', key='Ch4')]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Vertical',
                [
                    [sg.Slider(range=(0.5, 5), orientation='v', size=(5, 28), default_value=2, resolution=1.5,
                               tick_interval=1.5, disable_number_display=True, key='Horiz1'),
                     sg.Slider(range=(0.5, 5), orientation='v', size=(5, 28), default_value=2, resolution=1.5,
                               tick_interval=1.5, disable_number_display=True, key='Horiz2'),
                     sg.Slider(range=(0.5, 5), orientation='v', size=(5, 28), default_value=2, resolution=1.5,
                               tick_interval=1.5, disable_number_display=True, key='Horiz3'),
                     sg.Slider(range=(0.5, 5), orientation='v', size=(5, 28), default_value=2, resolution=1.5,
                               tick_interval=1.5, disable_number_display=True, key='Horiz4')],

                    [sg.Spin(values = ('10mV', '100mV', '1V'), size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Horiz1Scale'),
                     sg.Spin(values = ('10mV', '100mV', '1V'), size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Horiz2Scale'),
                     sg.Spin(values = ('10mV', '100mV', '1V'), size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Horiz3Scale'),
                     sg.Spin(values = ('10mV', '100mV', '1V'), size=(6,18), initial_value='1V',
                             font=('Helvetica', 8), key='Horiz4Scale')]
                ],
            title_color='yellow',
            border_width=10)],

            [sg.Frame('Triggers',
                [
                    [sg.Radio('Ch1', 'rd_triggers', key='Trig1'),
                     sg.Radio('Ch2', 'rd_triggers', key='Trig2'),
                     sg.Radio('Ch3', 'rd_triggers', key='Trig3'),
                     sg.Radio('Ch4', 'rd_triggers', key='Trig4')],

                    [sg.Text('Rise'), sg.Radio('', 'rd_risefall', key='Rise'),
                     sg.Text('Fall'), sg.Radio('', 'rd_risefall', key='Fall')],

                    [sg.Text('Low'), sg.Input(size=(3,18), default_text='0', key='LowL'),
                     sg.Spin(values=('V', 'mV'), initial_value='V', size=(3,18), key='LowLScale'),
                     sg.Text('High'), sg.Input(size=(3, 18), default_text='10', key='HighL'),
                     sg.Spin(values=('V','mV'), initial_value='V', size=(3,18), key='HighLScale')]
                ],
            title_color='yellow',
            border_width = 10)]
            
            [sg.Frame('Horizontal',
                [
                    [sg.Slider(range=(2, 10), orientation='h', size=(15, 25), default_value=2, resolution=2,
                               tick_interval=2, disable_number_display=True, key='HScale'),
                     sg.Spin(values=('1us', '100us', '1ms', '10ms', '100ms'), size=(12, 20), initial_value='1ms',
                             font=('Helvetica', 8), key='Horiz1Scale')]
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

        sg.Frame('Functions',
            [
                [sg.Table(values=([ch1fns, ch2fns, ch3fns, ch4fns]), headings=headings,
                          auto_size_columns=False, col_widths=[8,8,8,8,8,8], num_rows=4,
                          row_colors=[(0,'blue'), (1,'red'), (2,'green'), (3,'orange')],
                          hide_vertical_scroll=True, justification='center')
                ]
            ],
        pad=((200,0),(0,0)),
        title_color='yellow',
        border_width=10),
            
        sg.Text('Settings'), sg.Text("", size=(50, 7), key='settings')
      ]

choices = [[sg.Frame('Oscilloscope 4Channels', layout=options)]]


#---------------------random Sin wave Plot for demo-Alex's code will go here-----------------

fig = matplotlib.figure.Figure(figsize=(5, 4), dpi=100)
t = np.arange(0, 3, .01)
fig.add_subplot(111).plot(t, 2 * np.sin(2 * np.pi * t))



# ------------------------------- Beginning of Matplotlib helper code -----------------------

def draw_figure(canvas, figure):
    figure_canvas_agg = FigureCanvasTkAgg(figure, canvas)
    figure_canvas_agg.draw()
    figure_canvas_agg.get_tk_widget().pack(side='top', fill='both', expand=1)
    return figure_canvas_agg

# ------------------------------- Beginning of GUI CODE -------------------------------

# Create layout with two columns using precreated frames
layout = [[sg.Column(choices), (sg.Canvas(key='-CANVAS-'))]],fns

# create the form and show it without the plot
window = sg.Window('Demo Application - 4 Channel Oscilloscope', layout, location=(100, 100), finalize=True)

# add the plot to the window
fig_canvas_agg = draw_figure(window['-CANVAS-'].TKCanvas, fig)


while True:
    event, values = window.read()  # Read  values entered by user
    settings_list=values
    if event == sg.WIN_CLOSED:  # If window is closed by user terminate While Loop
        break
    if event == 'Submit':  # If submit button is clicked display chosen values
        window['settings'].update(settings_list)  # output the final string
    if event == 'Close':
        window['settings'].update(settings_list)
        break
# Close Window
window.close()
