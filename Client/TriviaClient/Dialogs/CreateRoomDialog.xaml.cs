﻿using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Navigation;
using System.Windows.Shapes;

namespace TriviaClient.Dialogs
{
    /// <summary>
    /// Interaction logic for CreateRoomDialog.xaml
    /// </summary>
    public partial class CreateRoomDialog : UserControl
    {
        public CreateRoomDialog()
        {
            InitializeComponent();

            maxPlayerBox.ItemsSource = Enumerable.Range(2, 99);
            timePerQuestionBox.ItemsSource = Enumerable.Range(10, 51);
            questionCountBox.ItemsSource = Enumerable.Range(1, 15);
        }
    }
}
