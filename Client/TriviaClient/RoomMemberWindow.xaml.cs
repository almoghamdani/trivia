﻿using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace TriviaClient
{
    public struct LeaveRoomRequest
    {
        public const int CODE = 11;
    }

    public struct LeaveRoomResponse
    {
        public int status;
    }

    /// <summary>
    /// Interaction logic for RoomMemberWindow.xaml
    /// </summary>
    public partial class RoomMemberWindow : Window
    {
        public RoomData room { get; set; }

        private CancellationTokenSource tokenSource = new CancellationTokenSource();

        public RoomMemberWindow()
        {
            InitializeComponent();
        }

        private void Window_Loaded(object sender, RoutedEventArgs e)
        {
            // Set title
            Title = $"Trivia - {room.name} - Room #{room.id}";

            // Set the action handler of the room preview
            roomPreview.Action = LeaveRoom;

            // Run the action in the background
            Task.Run(new Action(RefreshRoomData));
        }

        /// <summary>
        /// Refresh the room data
        /// </summary>
        /// <returns>
        /// None
        /// </returns>
        private async void RefreshRoomData()
        {
            GetRoomStateResponse res;
            byte[] buf;

            while (true)
            {
                // Exit if requested
                if (tokenSource.IsCancellationRequested) return;

                try
                {
                    // Send the get room state request
                    await Client.Send(GetRoomStateRequest.CODE, new byte[0]);

                    // Get from the server the room state
                    buf = await Client.Recv();

                    // Deserialize the response
                    res = JsonConvert.DeserializeObject<GetRoomStateResponse>(Encoding.UTF8.GetString(buf));

                    // If an error occurred, return to menu
                    if (res.status == 1)
                    {
                        Dispatcher.Invoke(() =>
                        {
                            // Close this window and the rooms window
                            new RoomsWindow().Show();
                            Close();
                        });

                        return;
                    }

                    // If a game has begun
                    if (res.hasGameBegun)
                    {
                        Dispatcher.Invoke(new Action(() =>
                        {
                            // Show the game window
                            new GameWindow()
                            {
                                AmountOfQuestions = room.questionCount,
                                TimePerQuestion = room.timePerQuestion
                            }.Show();
                            Close();
                        }));

                        return;
                    }

                    // Set the room preview data
                    roomPreview.Data = new RoomPreviewData
                    {
                        Id = room.id,
                        Name = room.name,
                        MaxPlayers = room.maxPlayers,
                        TimePerQuestion = res.answerTimeout,
                        QuestionCount = res.questionCount,
                        IsActive = !res.hasGameBegun,
                        ActionButtonText = "Leave Room",
                        ActionButtonEnabled = true,
                        Players = res.players
                    };

                    // Update the room preview in the main thread
                    roomPreview.Dispatcher.Invoke(new Action(() => { roomPreview.Update(); }));

                    // Wait 1 second
                    await Task.Delay(1000);
                }
                catch
                {
                    // Close this window and re-show the auth window to connect to the server
                    new AuthWindow().Show();
                    Close();

                    return;
                }
            }
        }

        /// <summary>
        /// Leaves the room
        /// </summary>
        /// <returns>
        /// None
        /// </returns>
        /// <param name="roomId">The id of the room</param>
        private async void LeaveRoom(int roomId)
        {
            LeaveRoomResponse res;
            byte[] buf;

            try
            {
                // Send the leave room request
                await Client.Send(LeaveRoomRequest.CODE, new byte[0]);

                // Get from the server the response
                buf = await Client.Recv();

                // Deserialize the response
                res = JsonConvert.DeserializeObject<LeaveRoomResponse>(Encoding.UTF8.GetString(buf));

                // Cancel the background action
                tokenSource.Cancel();

                // Close this window and the rooms window
                new RoomsWindow().Show();
                Close();
            } catch
            {
                // Cancel the background action
                tokenSource.Cancel();

                // Close this window and re-show the auth window to connect to the server
                new AuthWindow().Show();
                Close();
            }
        }
    }
}
