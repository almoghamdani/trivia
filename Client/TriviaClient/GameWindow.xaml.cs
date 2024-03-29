﻿using MaterialDesignThemes.Wpf;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Globalization;
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
using System.Windows.Shapes;
using System.Windows.Threading;

namespace TriviaClient
{
    public struct GetQuestionRequest
    {
        public const int CODE = 12;
    }

    public struct GetQuestionResponse
    {
        public int status;
        public string question;
        public SortedDictionary<int, string> answers;
    }

    public struct LeaveGameRequest
    {
        public const int CODE = 15;
    }

    public struct SubmitAnswerRequest
    {
        public const int CODE = 13;

        public int answerId;
    }

    public struct SubmitAnswerResponse
    {
        public int status;
        public int correctAnswerId;
    }

    public struct QuestionPreview
    {
        public string Question { get; set; }
        public List<string> Answers { get; set; }
        public int[] AnswerIndices { get; set; }
    }

    /// <summary>
    /// Interaction logic for GameWindow.xaml
    /// </summary>
    public partial class GameWindow : Window
    {
        public QuestionPreview CurrentQuestion { get; set; }
        public int SelectedAnswer { get; set; }
        public int AmountOfQuestions { get; set; }
        public int QuestionNumber { get; set; }
        public int TimePerQuestion { get; set; }
        public int RemainingTime { get; set; }

        private Random rnd = new Random();
        private RadioButton[] radioButtons { get; set; }
        private DispatcherTimer timer { get; set; }

        public GameWindow()
        {
            SelectedAnswer = -1;
            QuestionNumber = 1;

            InitializeComponent();

            radioButtons = new[] { radioBtn0, radioBtn1, radioBtn2, radioBtn3 };
        }

        private void Update()
        {
            DataContext = null;
            DataContext = this;
        }

        /// <summary>
        /// Gets random answer idices
        /// </summary>
        /// <returns>
        /// Array with random answer indices
        /// </returns>
        private int[] RandomAnswerIndices()
        {
            int[] indices = { 0, 1, 2, 3 };

            return indices.OrderBy(x => rnd.Next()).ToArray();
        }

        /// <summary>
        /// Gets the answers sorted by their indices
        /// </summary>
        /// <returns>
        /// The list of answers
        /// </returns>
        /// <param name="answers">The dict of answers</param>
        /// <param name="answerIndices">The indices of the answers</param>
        private List<string> GetAnswersByIndices(SortedDictionary<int, string> answers, int[] answerIndices)
        {
            List<string> answersList = new List<string>();

            // For each answer index, add the answer to the list of answers
            foreach (int answerIdx in answerIndices)
            {
                answersList.Add(answers[answerIdx]);
            }

            return answersList;
        }

        /// <summary>
        /// Gets the current question
        /// </summary>
        /// <returns>
        /// Task
        /// </returns>
        private async Task GetQuestion()
        {
            GetQuestionResponse res;
            byte[] buf;

            int[] answerIndices = RandomAnswerIndices();

            try
            {
                // Send the get question request
                await Client.Send(GetQuestionRequest.CODE, new byte[0]);

                // Get from the server the response
                buf = await Client.Recv();

                // Deserialize the response
                res = JsonConvert.DeserializeObject<GetQuestionResponse>(Encoding.UTF8.GetString(buf));

                // Set the current question
                CurrentQuestion = new QuestionPreview
                {
                    Question = res.question,
                    AnswerIndices = answerIndices,
                    Answers = GetAnswersByIndices(res.answers, answerIndices)
                };
            } catch
            {
                Dispatcher.Invoke(() =>
                {
                    // Close this window and re-show the auth window to connect to the server
                    new AuthWindow().Show();
                    Close();
                });
            }
        }

        private async void Window_Loaded(object sender, RoutedEventArgs e)
        {
            await GetQuestion();

            // Set the initial remaining time
            RemainingTime = TimePerQuestion;

            // Start the timer
            timer = new DispatcherTimer(DispatcherPriority.Normal, Application.Current.Dispatcher);
            timer.Interval = TimeSpan.FromSeconds(1);
            timer.Tick += TimerTick;
            timer.Start();

            Update();
        }

        private async void TimerTick(object sneder, EventArgs e)
        {
            // If the time ran out, send an empty answer
            if (RemainingTime == 0)
            {
                // Stop the timer
                timer.Stop();

                DisableRadioButtons();

                SubmitAnswerRequest req = new SubmitAnswerRequest
                {
                    answerId = 5
                };
                SubmitAnswerResponse res;
                byte[] buf;

                int[] answerIndices = RandomAnswerIndices();

                try
                {
                    // Send the submit answer request
                    await Client.Send(SubmitAnswerRequest.CODE, Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(req)));

                    // Get from the server the response
                    buf = await Client.Recv();

                    // Deserialize the response
                    res = JsonConvert.DeserializeObject<SubmitAnswerResponse>(Encoding.UTF8.GetString(buf));

                    // If an error occurred
                    if (res.status == 1)
                    {
                        // Show error
                        await DialogHost.Show(new Dialogs.MessageDialog { Message = "Unable to submit answer!" });

                        throw new Exception();
                    }
                    else
                    {
                        // Highlight the correct answer
                        GetRadioButtonForAnswer(res.correctAnswerId).Foreground = Brushes.LightGreen;

                        // If was wrong, highlight in red the wrong answer
                        if (SelectedAnswer != -1 && res.correctAnswerId != CurrentQuestion.AnswerIndices[SelectedAnswer])
                        {
                            radioButtons[SelectedAnswer].Foreground = Brushes.Red;
                        }
                    }
                } catch
                {
                    Dispatcher.Invoke(() =>
                    {
                        // Close this window and re-show the auth window to connect to the server
                        new AuthWindow().Show();
                        Close();
                    });

                    return;
                }
                
                // Enable the button
                submitNextBtn.IsEnabled = true;

                // Change the button to the next button
                submitNextBtn.Content = "Next";
            } else
            {
                RemainingTime -= 1;
            }

            Update();
        }

        private void RadioButton_Checked(object sender, RoutedEventArgs e)
        {
            RadioButton btn = sender as RadioButton;

            // Set the new selected answer and update
            SelectedAnswer = int.Parse((string)btn.Tag);
        }

        private async void leaveGameBtn_Click(object sender, RoutedEventArgs e)
        {
            byte[] buf;

            try
            {
                // Send the leave game request
                await Client.Send(LeaveGameRequest.CODE, new byte[0]);

                // Get from the server the response
                buf = await Client.Recv();

                // Show the rooms window
                new RoomsWindow().Show();
                Close();
            } catch
            {
                // Close this window and re-show the auth window to connect to the server
                new AuthWindow().Show();
                Close();
            }
            
        }

        /// <summary>
        /// Gets the radio button for an answer
        /// </summary>
        /// <returns>
        /// The wanted radio button
        /// </returns>
        /// <param name="answerId">The id of the answer</param>
        private RadioButton GetRadioButtonForAnswer(int answerId)
        {
            int realIndex = Array.IndexOf(CurrentQuestion.AnswerIndices, answerId);

            return radioButtons[realIndex];
        }

        /// <summary>
        /// Submits the answer
        /// </summary>
        /// <returns>
        /// Task
        /// </returns>
        private async Task SubmitAnswer()
        {
            SubmitAnswerRequest req = new SubmitAnswerRequest
            {
                answerId = CurrentQuestion.AnswerIndices[SelectedAnswer]
            };
            SubmitAnswerResponse res;
            byte[] buf;

            int[] answerIndices = RandomAnswerIndices();

            try
            {
                // Send the submit answer request
                await Client.Send(SubmitAnswerRequest.CODE, Encoding.UTF8.GetBytes(JsonConvert.SerializeObject(req)));

                // Get from the server the response
                buf = await Client.Recv();

                // Deserialize the response
                res = JsonConvert.DeserializeObject<SubmitAnswerResponse>(Encoding.UTF8.GetString(buf));

                // If an error occurred
                if (res.status == 1)
                {
                    // Show error
                    await DialogHost.Show(new Dialogs.MessageDialog { Message = "Unable to submit answer!" });

                    throw new Exception();
                }
                else
                {
                    // Highlight the correct answer
                    GetRadioButtonForAnswer(res.correctAnswerId).Foreground = Brushes.LightGreen;

                    // If was wrong, highlight in red the wrong answer
                    if (res.correctAnswerId != CurrentQuestion.AnswerIndices[SelectedAnswer])
                    {
                        radioButtons[SelectedAnswer].Foreground = Brushes.Red;
                    }
                }
            } catch
            {
                Dispatcher.Invoke(() =>
                {
                    // Close this window and re-show the auth window to connect to the server
                    new AuthWindow().Show();
                    Close();
                });
            }
        }

        private void UnselectRadioButtons()
        {
            SelectedAnswer = -1;

            // Unselect all buttons
            foreach (RadioButton btn in radioButtons)
            {
                btn.IsChecked = false;
                btn.Foreground = Brushes.White;
            }
        }

        private void DisableRadioButtons()
        {
            // Disable all buttons
            foreach (RadioButton btn in radioButtons)
            {
                btn.IsHitTestVisible = false;
            }
        }

        private void EnableRadioButtons()
        {
            // Enable all buttons
            foreach (RadioButton btn in radioButtons)
            {
                btn.IsHitTestVisible = true;
            }
        }

        private async void submitNextBtn_Click(object sender, RoutedEventArgs e)
        {
            Button btn = sender as Button;

            // If nothing selected, return
            if ((string)btn.Content == "Submit" && SelectedAnswer == -1)
            {
                return;
            }

            // Disable the button
            btn.IsEnabled = false;

            // Submit
            if ((string)btn.Content == "Submit")
            {
                // Stop the timer
                timer.Stop();

                DisableRadioButtons();

                try
                {
                    await SubmitAnswer();
                } catch {
                    // Enable the button
                    btn.IsEnabled = true;

                    EnableRadioButtons();

                    return;
                }

                // Enable the button
                btn.IsEnabled = true;

                // Change the button to the next button
                btn.Content = "Next";
            } else
            {
                // If finished the last question, show the game results window
                if (QuestionNumber == AmountOfQuestions)
                {
                    new GameResultsWindow().Show();
                    Close();

                    return;
                }

                UnselectRadioButtons();
                EnableRadioButtons();

                // Get the next question
                await GetQuestion();

                // Increase the question number
                QuestionNumber++;

                // Reset remaining time
                RemainingTime = TimePerQuestion;

                // Start the timer
                timer = new DispatcherTimer(DispatcherPriority.Normal, Application.Current.Dispatcher);
                timer.Interval = TimeSpan.FromSeconds(1);
                timer.Tick += TimerTick;
                timer.Start();

                // Update UI
                Update();

                // Enable the button
                btn.IsEnabled = true;

                // Change the button to the next button
                btn.Content = "Submit";
            }
        }
    }
}
